#include "CustomPlugin.h"
#include "QGCLoggingCategory.h"
#include "QGCPalette.h"
#include "QGCMAVLink.h"
#include "AppSettings.h"
#include "Vehicle.h"
#include "RocketTelemetryFactGroup.h"
#include "FirmwarePlugin/CustomFirmwarePlugin.h"
#include "safety/SafetyKernel.h"
#include "protocol/M130Dialect.h"
#include "protocol/ProtocolVersion.h"
#include "protocol/generated/M130DialectTable.generated.h"
#include "protocol/generated/M130Messages.generated.h"
#include "telemetry/M130GncStateFactGroup.h"
#include "logging/FlightDataRecorder.h"
#include "access/AccessController.h"
#include "views/MissionController.h"
#include "tuning/TuningController.h"
#include "analysis/AnalysisController.h"
#include "safety/FlightPhase.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <sstream>

#include <QtCore/QApplicationStatic>
#include <QtCore/QByteArray>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

QGC_LOGGING_CATEGORY(CustomLog, "Custom.M130Plugin")

Q_APPLICATION_STATIC(CustomPlugin, _customPluginInstance);

// ============================================================================
// CustomFlyViewOptions
// ============================================================================

CustomFlyViewOptions::CustomFlyViewOptions(QGCOptions *options, QObject *parent)
    : QGCFlyViewOptions(options, parent)
{
}

// ============================================================================
// CustomOptions
// ============================================================================

CustomOptions::CustomOptions(CustomPlugin *plugin, QObject *parent)
    : QGCOptions(parent)
    , _plugin(plugin)
    , _flyViewOptions(new CustomFlyViewOptions(this, this))
{
    Q_CHECK_PTR(_plugin);
}

// ============================================================================
// CustomPlugin
// ============================================================================

CustomPlugin::CustomPlugin(QObject *parent)
    : QGCCorePlugin(parent)
    , _options(new CustomOptions(this, this))
    , _safetyKernel(new m130::gui::SafetyKernel(this))
    , _access(new m130::gui::AccessController(this))
    , _mission(new m130::gui::MissionController(this))
    , _tuning(new m130::gui::TuningController(this))
    , _analysis(new m130::gui::AnalysisController(this))
{
    qCDebug(CustomLog) << "M130 GCS Plugin initializing";
    _showAdvancedUI = false;
    _safetyKernel->installDefaults();
    _access->installDefaultUsers();
    _mission->installDefaultChecklist();
    _tuning->installDefaults();
    _analysis->installDefaultSeries();

    // Safety Kernel → Tuning: route mission-phase changes so the tuning gate
    // can reject weight changes in BOOST/CRUISE without a step-up.
    (void) connect(_safetyKernel, &m130::gui::SafetyKernel::phaseChanged,
                   this, [this]{ _tuning->setCurrentPhase(_safetyKernel->currentPhaseInt()); });

    // Access → Tuning: step-up freshness signals whether the operator has
    // re-authenticated recently enough for critical parameter edits.
    (void) connect(_access, &m130::gui::AccessController::sessionChanged,
                   this, [this]{
                       _tuning->setStepUpFresh(!_access->stepUpRequired());
                   });

    _initFlightDataRecorder();
    (void) connect(this, &QGCCorePlugin::showAdvancedUIChanged, this, &CustomPlugin::_advancedChanged);
}

CustomPlugin::~CustomPlugin()
{
    if (_fdr) _fdr->close();
}

// Open the Flight Data Recorder under the directory pointed to by the env var
// M130_FDR_DIR. The directory is created if it does not exist. Each session
// lands under its own base path: `<dir>/m130_<YYYYmmdd_HHMMSS>`. When the env
// var is unset, no recorder is created — pure observation-only mode.
void CustomPlugin::_initFlightDataRecorder()
{
    const char *dir_env = std::getenv("M130_FDR_DIR");
    if (!dir_env || *dir_env == '\0') return;

    std::error_code ec;
    std::filesystem::create_directories(dir_env, ec);
    if (ec) {
        qCWarning(CustomLog) << "M130_FDR_DIR cannot be created:" << QString::fromStdString(ec.message());
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto tt  = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#ifdef _WIN32
    gmtime_s(&tm_buf, &tt);
#else
    gmtime_r(&tt, &tm_buf);
#endif
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tm_buf);

    const std::string base = (std::filesystem::path(dir_env) /
                              (std::string("m130_") + stamp)).string();
    _fdr = std::make_unique<m130::logging::FlightDataRecorder>();
    if (!_fdr->open(base)) {
        qCWarning(CustomLog) << "FlightDataRecorder failed to open" << QString::fromStdString(base);
        _fdr.reset();
    } else {
        qCInfo(CustomLog) << "FlightDataRecorder active at" << QString::fromStdString(base);
    }
}

void CustomPlugin::_recordRawFrame(const mavlink_message_t &message, std::uint8_t channel)
{
    if (!_fdr) return;
    m130::logging::RawFrame frame;
    frame.channel = channel;
    frame.msg_id  = static_cast<std::uint16_t>(message.msgid);
    frame.payload.assign(
        reinterpret_cast<const std::uint8_t*>(_MAV_PAYLOAD(&message)),
        reinterpret_cast<const std::uint8_t*>(_MAV_PAYLOAD(&message)) + message.len);
    _fdr->appendRaw(frame);
}

QGCCorePlugin *CustomPlugin::instance()
{
    return _customPluginInstance();
}

void CustomPlugin::cleanup()
{
    if (_qmlEngine) {
        _qmlEngine->removeUrlInterceptor(_selector);
    }
    delete _selector;
}

void CustomPlugin::_advancedChanged(bool changed)
{
    emit _options->showFirmwareUpgradeChanged(changed);
}

// ============================================================================
// mavlinkMessage — يعترض DEBUG_FLOAT_ARRAY ويمرره لـ RocketTelemetryFactGroup
// ============================================================================

bool CustomPlugin::mavlinkMessage(Vehicle *vehicle, LinkInterface * /*link*/, const mavlink_message_t &message)
{
    // Feed the Safety Kernel watchdog on every heartbeat and on each valid
    // RktGNC telemetry block so staleness alerts clear automatically.
    if (_safetyKernel) {
        if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
            _safetyKernel->feed(QStringLiteral("heartbeat"));
        }
    }

    // ------------------------------------------------------------------
    // Pillar 4 — Flight Data Recorder capture for any M130 dialect id and
    // for HEARTBEAT so the raw track has full context. The recorder is a
    // no-op if not enabled.
    // ------------------------------------------------------------------
    if (_fdr && (m130::protocol::isM130Id(message.msgid) ||
                 message.msgid == MAVLINK_MSG_ID_HEARTBEAT)) {
        _recordRawFrame(message, /*channel=*/0);
    }

    // ------------------------------------------------------------------
    // Pillar 2 — dispatch M130 custom dialect messages (42000-42255).
    // Messages are decoded from their MAVLink v2 payload via the generated
    // structs; CRC validation happens one layer up in the QGC mavlink stack.
    // ------------------------------------------------------------------
    if (m130::protocol::isM130Id(message.msgid)) {
        _dispatchM130Message(vehicle, message);
        // Fall through: we do NOT return early so MAVLink Inspector still
        // sees the message and other plugins can observe it.
    }

    if (message.msgid == MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        mavlink_debug_float_array_t dbg;
        mavlink_msg_debug_float_array_decode(&message, &dbg);

        if (dbg.array_id == RocketTelemetryFactGroup::ROCKET_ARRAY_ID) {
            FactGroup *fg = vehicle->getFactGroup(QStringLiteral("rocket"));
            if (fg) {
                fg->handleMessage(vehicle, message);
            }
            if (_safetyKernel) {
                _safetyKernel->feed(QStringLiteral("gnc_state"));
                // Envelope checks on the safety-critical fields. Indices match
                // RocketTelemetryFactGroup / DEBUG_FLOAT_ARRAY.hpp contract and
                // docs/safety/SafetyEnvelope.md.
                //   idx  4: rollRateCmd (unused for envelope today)
                //   idx 15: altitude    (not yet in default envelope)
                //   idx 18: phi (roll)
                //   idx 21: alpha_est
                //   idx 38: mpc_solve_us
                _safetyKernel->evaluateSample(QStringLiteral("phi"),       dbg.data[18]);
                _safetyKernel->evaluateSample(QStringLiteral("alpha_est"), dbg.data[21]);
                _safetyKernel->evaluateSample(QStringLiteral("mpc_solve_us"), dbg.data[38]);
            }
            // لا نوقف المعالجة — اسمح لـ MAVLink Inspector بعرضها أيضاً
        }
    }
    return true;
}

// ============================================================================
// _dispatchM130Message — يفك ترميز رسائل m130 الخاصة باستخدام الكود المُولَّد
// من m130.xml ثم يوزّعها على الطبقات المستهلكة (FactGroups + Safety Kernel).
// ============================================================================

void CustomPlugin::_dispatchM130Message(Vehicle *vehicle, const mavlink_message_t &message)
{
    using namespace m130::protocol::gen;

    // MAVLink v2 keeps the payload inside the same packet after CRC framing
    // has been stripped. `mavlink_message_t::payload64` is 8-byte aligned.
    const std::uint8_t* payload = reinterpret_cast<const std::uint8_t*>(
        _MAV_PAYLOAD(&message));
    const std::size_t len = static_cast<std::size_t>(message.len);

    switch (message.msgid) {
    case M130HeartbeatExtended::kMsgId: {
        M130HeartbeatExtended hb;
        if (!M130HeartbeatExtended::unpack(payload, len, hb)) break;
        const auto peer = m130::protocol::ProtocolVersion::unpack(hb.protocol_version);
        const auto compat = m130::protocol::checkCompat(peer);
        if (_safetyKernel) {
            _safetyKernel->feed(QStringLiteral("heartbeat"));
            const auto level = m130::protocol::alertLevelFor(compat.compat);
            if (level != m130::safety::AlertLevel::None) {
                // Surface the compat delta on the master caution bus. Emergency
                // on MajorMismatch tells the operator that telemetry from this
                // peer cannot be trusted (REQ-M130-GCS-SAFE-005, §3.7 of the
                // deep-analysis report).
                _safetyKernel->aggregator().alerts().raise(
                    std::string(m130::protocol::kCompatAlertId),
                    level,
                    "Protocol version mismatch",
                    compat.message);
            } else {
                // Peer moved back onto a compatible version — clear any stale
                // advisory so the master-caution bus returns to green.
                _safetyKernel->aggregator().alerts().clear(
                    std::string(m130::protocol::kCompatAlertId));
            }
        }
        break;
    }
    case M130GncState::kMsgId: {
        M130GncState gnc;
        if (!M130GncState::unpack(payload, len, gnc)) break;
        if (auto *fwp = qobject_cast<CustomFirmwarePlugin*>(vehicle ? vehicle->firmwarePlugin() : nullptr)) {
            if (auto *fg = fwp->m130GncFactGroup()) {
                fg->applyState(gnc);
            }
        }
        if (_safetyKernel) {
            _safetyKernel->feed(QStringLiteral("gnc_state"));
            _safetyKernel->evaluateSample(QStringLiteral("phi"), gnc.phi);
            _safetyKernel->evaluateSample(QStringLiteral("alpha_est"), gnc.alpha_est);
        }
        // Phase B — feed Pillar 7 live analysis and Pillar 5 IIP.
        if (_analysis) {
            const auto tMs = static_cast<qint64>(gnc.time_usec / 1000);
            _analysis->push(QStringLiteral("phi"),          tMs, gnc.phi);
            _analysis->push(QStringLiteral("theta"),        tMs, gnc.theta);
            _analysis->push(QStringLiteral("psi"),          tMs, gnc.psi);
            _analysis->push(QStringLiteral("q_dyn"),        tMs, gnc.q_dyn);
            _analysis->push(QStringLiteral("altitude"),     tMs, gnc.altitude);
            _analysis->push(QStringLiteral("airspeed"),     tMs, gnc.airspeed);
            _analysis->push(QStringLiteral("alpha_est"),    tMs, gnc.alpha_est);
        }
        if (_mission) {
            // GNC frame: downrange + crossrange are horizontal; altitude is
            // AGL (up-positive). Velocity components are not carried on this
            // message yet — pass zero so IIP degrades to a drop prediction.
            _mission->updateIip(gnc.pos_downrange, gnc.pos_crossrange, gnc.altitude,
                                0.0, 0.0, 0.0);
        }
        // Pillar 4 — structured track: persist a small subset of decoded
        // GNC fields so post-flight analysis does not require reparsing the
        // raw payload. Full schema lives in docs/design/FlightDataRecorder.md.
        if (_fdr) {
            m130::logging::StructuredSample s;
            s.timestamp_us = gnc.time_usec;
            s.msg_id       = M130GncState::kMsgId;
            s.msg_name     = M130GncState::kName;
            auto push = [&s](const char *k, double v) {
                std::ostringstream os; os.precision(9); os << v;
                s.numeric.emplace_back(k, os.str());
            };
            push("stage",          gnc.stage);
            push("launched",       gnc.launched);
            push("mode",           gnc.mode);
            push("q_dyn",          gnc.q_dyn);
            push("altitude",       gnc.altitude);
            push("airspeed",       gnc.airspeed);
            push("phi",            gnc.phi);
            push("theta",          gnc.theta);
            push("psi",            gnc.psi);
            push("alpha_est",      gnc.alpha_est);
            push("pos_downrange",  gnc.pos_downrange);
            push("pos_crossrange", gnc.pos_crossrange);
            _fdr->appendStructured(s);
        }
        break;
    }
    default:
        // Known dialect id but no handler yet (tracked in ICD-MAVLink.md §3.2).
        // Deliberately no-op so the Message Router can still tee into FDR.
        break;
    }
}

// ============================================================================
// adjustSettingMetaData — نخفي إعدادات غير ذات صلة بالصاروخ
// ============================================================================

void CustomPlugin::adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData, bool &userVisible)
{
    QGCCorePlugin::adjustSettingMetaData(settingsGroup, metaData, userVisible);

    if (settingsGroup == AppSettings::settingsGroup) {
        if (metaData.name() == AppSettings::offlineEditingFirmwareClassName) {
            metaData.setRawDefaultValue(QGCMAVLink::FirmwareClassPX4);
            userVisible = false;
        } else if (metaData.name() == AppSettings::offlineEditingVehicleClassName) {
            metaData.setRawDefaultValue(QGCMAVLink::VehicleClassGeneric);
            userVisible = false;
        }
    }
}

// ============================================================================
// paletteOverride — سكيمة Aerospace Dark للمحطة الأرضية
//
// اللون الأساسي: #00D4AA (تيل فلوري)
// الخلفية:      #0D1117 (أسود مزرق عميق)
// ============================================================================

void CustomPlugin::paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo)
{
    if (colorName == QStringLiteral("window")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#0D1117");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#0D1117");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#F0F4F8");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#E8EDF2");
    } else if (colorName == QStringLiteral("windowShade")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#161B22");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#161B22");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#E1E8EF");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#D0D8E0");
    } else if (colorName == QStringLiteral("windowShadeDark")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#090D12");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#090D12");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#C8D4DC");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#B8C4CC");
    } else if (colorName == QStringLiteral("text")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#E8EAF0");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#5A6475");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#1A2332");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#8090A0");
    } else if (colorName == QStringLiteral("warningText")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#FF2D55");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#FF2D55");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#D0002A");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#D0002A");
    } else if (colorName == QStringLiteral("button")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#1C2430");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#1C2430");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#FFFFFF");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#F0F4F8");
    } else if (colorName == QStringLiteral("buttonText")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#E8EAF0");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#5A6475");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#1A2332");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#8090A0");
    } else if (colorName == QStringLiteral("buttonHighlight")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00D4AA");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#1C2430");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#00AA88");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#C8D4DC");
    } else if (colorName == QStringLiteral("buttonHighlightText")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#0D1117");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#5A6475");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#FFFFFF");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#8090A0");
    } else if (colorName == QStringLiteral("primaryButton")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00D4AA");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#1C2430");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#00AA88");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#C8D4DC");
    } else if (colorName == QStringLiteral("primaryButtonText")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#0D1117");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#E8EAF0");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#FFFFFF");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#8090A0");
    } else if (colorName == QStringLiteral("colorGreen")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00FF7F");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#00CC66");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#009944");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#007733");
    } else if (colorName == QStringLiteral("colorOrange")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#FF8C00");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#CC7000");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#B85000");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#994000");
    } else if (colorName == QStringLiteral("colorRed")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#FF2D55");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#CC2244");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#D0002A");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#A00020");
    } else if (colorName == QStringLiteral("mapButton")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#0D1117");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#3A4455");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#1A2332");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#3A4455");
    } else if (colorName == QStringLiteral("mapButtonHighlight")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00D4AA");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#3A4455");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#FF8C00");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#3A4455");
    } else if (colorName == QStringLiteral("mapIndicator")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00D4AA");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#3A4455");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#FF8C00");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#3A4455");
    } else if (colorName == QStringLiteral("brandingPurple")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#00D4AA");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#00D4AA");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#00D4AA");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#00D4AA");
    } else if (colorName == QStringLiteral("brandingBlue")) {
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupEnabled]   = QColor("#0077FF");
        colorInfo[QGCPalette::Dark][QGCPalette::ColorGroupDisabled]  = QColor("#0055CC");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupEnabled]  = QColor("#0055CC");
        colorInfo[QGCPalette::Light][QGCPalette::ColorGroupDisabled] = QColor("#0044AA");
    }
}

// ============================================================================
// createQmlApplicationEngine — يُضيف import paths ويُثبّت URL interceptor
// ============================================================================

QQmlApplicationEngine *CustomPlugin::createQmlApplicationEngine(QObject *parent)
{
    _qmlEngine = QGCCorePlugin::createQmlApplicationEngine(parent);

    _selector = new CustomOverrideInterceptor();
    _qmlEngine->addUrlInterceptor(_selector);

    // Publish the Safety Kernel to QML so MasterCautionLight, AlertBanner,
    // and console views can bind to its properties (REQ-M130-GCS-UI-001/002).
    auto *ctx = _qmlEngine->rootContext();
    if (_safetyKernel) ctx->setContextProperty(QStringLiteral("m130SafetyKernel"), _safetyKernel);
    if (_access)       ctx->setContextProperty(QStringLiteral("m130Access"),       _access);
    if (_mission)      ctx->setContextProperty(QStringLiteral("m130Mission"),      _mission);
    if (_tuning)       ctx->setContextProperty(QStringLiteral("m130Tuning"),       _tuning);
    if (_analysis)     ctx->setContextProperty(QStringLiteral("m130Analysis"),     _analysis);

    return _qmlEngine;
}

// ============================================================================
// CustomOverrideInterceptor
// ============================================================================

CustomOverrideInterceptor::CustomOverrideInterceptor()
    : QQmlAbstractUrlInterceptor()
{
}

QUrl CustomOverrideInterceptor::intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type)
{
    switch (type) {
    case QQmlAbstractUrlInterceptor::QmlFile:
    case QQmlAbstractUrlInterceptor::UrlString:
        if (url.scheme() == QStringLiteral("qrc")) {
            const QString origPath = url.path();
            const QString overrideRes = QStringLiteral(":/Custom%1").arg(origPath);
            if (QFile::exists(overrideRes)) {
                const QString relPath = overrideRes.mid(2);
                QUrl result;
                result.setScheme(QStringLiteral("qrc"));
                result.setPath('/' + relPath);
                return result;
            }
        }
        break;
    default:
        break;
    }

    return url;
}

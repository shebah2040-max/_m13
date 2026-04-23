#pragma once

#include <QtCore/QTranslator>
#include <QtQml/QQmlAbstractUrlInterceptor>

#include "QGCCorePlugin.h"
#include "QGCOptions.h"

#include <memory>

class CustomPlugin;
class QQmlApplicationEngine;

namespace m130::gui {
class SafetyKernel;
class AccessController;
class MissionController;
class TuningController;
class AnalysisController;
}
namespace m130::logging {
class FlightDataRecorder;
}

Q_DECLARE_LOGGING_CATEGORY(CustomLog)

// ============================================================================
// CustomFlyViewOptions — يخفي عناصر UI غير ضرورية للصاروخ
// ============================================================================

class CustomFlyViewOptions : public QGCFlyViewOptions
{
    Q_OBJECT

public:
    explicit CustomFlyViewOptions(QGCOptions *options, QObject *parent = nullptr);

    // لا حاجة لـ instrument panel القياسي — لدينا HUD مخصص
    bool showInstrumentPanel() const final { return false; }
    // الصاروخ مركبة واحدة دائماً
    bool showMultiVehicleList() const final { return false; }
};

// ============================================================================
// CustomOptions
// ============================================================================

class CustomOptions : public QGCOptions
{
    Q_OBJECT

public:
    explicit CustomOptions(CustomPlugin *plugin, QObject *parent = nullptr);

    bool showFirmwareUpgrade() const final { return _plugin->showAdvancedUI(); }
    QGCFlyViewOptions *flyViewOptions() const final { return _flyViewOptions; }

private:
    QGCCorePlugin *_plugin = nullptr;
    CustomFlyViewOptions *_flyViewOptions = nullptr;
};

// ============================================================================
// CustomPlugin — نقطة دخول المحطة الأرضية M130
// ============================================================================

class CustomPlugin : public QGCCorePlugin
{
    Q_OBJECT

public:
    explicit CustomPlugin(QObject *parent = nullptr);
    ~CustomPlugin() override;

    static QGCCorePlugin *instance();

    /// Safety Kernel accessor (test seam + CustomFirmwarePlugin integration).
    m130::gui::SafetyKernel *safetyKernel() const noexcept { return _safetyKernel; }

    /// Flight Data Recorder accessor. May be null if recording is not enabled.
    m130::logging::FlightDataRecorder *flightDataRecorder() const noexcept { return _fdr.get(); }

    /// Phase B — Qt-bound facades for Pillars 5/6/7.
    m130::gui::AccessController   *accessController()   const noexcept { return _access; }
    m130::gui::MissionController  *missionController()  const noexcept { return _mission; }
    m130::gui::TuningController   *tuningController()   const noexcept { return _tuning; }
    m130::gui::AnalysisController *analysisController() const noexcept { return _analysis; }

    // Overrides
    void cleanup() final;
    QGCOptions *options() final { return _options; }
    void adjustSettingMetaData(const QString &settingsGroup, FactMetaData &metaData, bool &userVisible) final;
    void paletteOverride(const QString &colorName, QGCPalette::PaletteColorInfo_t &colorInfo) final;
    QQmlApplicationEngine *createQmlApplicationEngine(QObject *parent) final;

    // DEBUG_FLOAT_ARRAY interceptor — يمرر الرسائل لـ RocketTelemetryFactGroup
    bool mavlinkMessage(Vehicle *vehicle, LinkInterface *link, const mavlink_message_t &message) final;

private slots:
    void _advancedChanged(bool advanced);

private:
    void _dispatchM130Message(Vehicle *vehicle, const mavlink_message_t &message);
    void _initFlightDataRecorder();
    void _recordRawFrame(const mavlink_message_t &message, std::uint8_t channel);

private:
    CustomOptions *_options = nullptr;
    QQmlApplicationEngine *_qmlEngine = nullptr;
    class CustomOverrideInterceptor *_selector = nullptr;
    m130::gui::SafetyKernel     *_safetyKernel = nullptr;
    m130::gui::AccessController *_access       = nullptr;
    m130::gui::MissionController*_mission      = nullptr;
    m130::gui::TuningController *_tuning       = nullptr;
    m130::gui::AnalysisController *_analysis   = nullptr;
    std::unique_ptr<m130::logging::FlightDataRecorder> _fdr;
};

// ============================================================================
// CustomOverrideInterceptor — يتيح override ملفات QML الأساسية
// ============================================================================

class CustomOverrideInterceptor : public QQmlAbstractUrlInterceptor
{
public:
    CustomOverrideInterceptor();
    QUrl intercept(const QUrl &url, QQmlAbstractUrlInterceptor::DataType type) final;
};

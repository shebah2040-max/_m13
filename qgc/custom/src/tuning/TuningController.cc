#include "TuningController.h"

#include <chrono>
#include <utility>

using m130::tuning::MpcTuningModel;
using m130::tuning::SetResult;
using m130::tuning::TuningSnapshot;

namespace m130::gui {

namespace {
quint64 nowMsEpoch()
{
    using namespace std::chrono;
    return static_cast<quint64>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

// FlightPhase ordinals mirrored from src/safety/FlightPhase.h. Kept as ints
// here so the TuningController has no build-time dependency on Safety.
constexpr int kPhaseUnknown    = 0;
constexpr int kPhaseIdle       = 1;
constexpr int kPhasePrelaunch  = 2;
constexpr int kPhaseArmed      = 3;
// constexpr int kPhaseBoost    = 4; // disallowed
// constexpr int kPhaseCruise   = 5; // disallowed
constexpr int kPhaseTerminal   = 6;  // allowed (tuning rollback during reentry)
constexpr int kPhaseLanded     = 7;
constexpr int kPhaseAbort      = 8;

bool phaseAllowsTuning(int p) noexcept
{
    switch (p) {
    case kPhaseUnknown:
    case kPhaseIdle:
    case kPhasePrelaunch:
    case kPhaseArmed:
    case kPhaseLanded:
    case kPhaseAbort:
    case kPhaseTerminal:
        return true;
    default:
        return false;
    }
}
} // namespace

// ---------------- TuningParamModel ----------------

TuningParamModel::TuningParamModel(const MpcTuningModel* core, QObject* parent)
    : QAbstractListModel(parent), _core(core)
{
    rebuild();
}

void TuningParamModel::rebuild()
{
    beginResetModel();
    _names = _core ? _core->names() : std::vector<std::string>{};
    endResetModel();
}

int TuningParamModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(_names.size());
}

QVariant TuningParamModel::data(const QModelIndex& index, int role) const
{
    if (!_core || !index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(_names.size())) {
        return {};
    }
    const auto& name = _names[static_cast<std::size_t>(index.row())];
    const auto spec  = _core->spec(name);
    const auto value = _core->value(name);
    if (!spec || !value) return {};
    switch (role) {
    case NameRole:        return QString::fromStdString(name);
    case ValueRole:       return *value;
    case MinRole:         return spec->min_value;
    case MaxRole:         return spec->max_value;
    case DefaultRole:     return spec->default_value;
    case UnitRole:        return QString::fromStdString(spec->unit);
    case DescriptionRole: return QString::fromStdString(spec->description);
    }
    return {};
}

QHash<int, QByteArray> TuningParamModel::roleNames() const
{
    return {
        {NameRole,        "paramName"},
        {ValueRole,       "value"},
        {MinRole,         "minValue"},
        {MaxRole,         "maxValue"},
        {DefaultRole,     "defaultValue"},
        {UnitRole,        "unit"},
        {DescriptionRole, "description"},
    };
}

// ---------------- TuningController ----------------

TuningController::TuningController(QObject* parent)
    : QObject(parent), _model(new TuningParamModel(&_core, this))
{
    _core.setClock([]{ return nowMsEpoch(); });
    _core.setSafetyGate([this](std::string_view name, double cur, double req) {
        return _safetyGate(std::string(name), cur, req);
    });
}

TuningController::~TuningController() = default;

void TuningController::setCurrentPhase(int phase)
{
    if (phase == _phase) return;
    _phase = phase;
    emit currentPhaseChanged();
}

void TuningController::setStepUpFresh(bool fresh)
{
    if (fresh == _step_up_fresh) return;
    _step_up_fresh = fresh;
    emit stepUpFreshChanged();
}

void TuningController::installDefaults()
{
    _core.registerStandardMpcWeights();
    _model->rebuild();
}

QVariantMap TuningController::snapshotToVariant(const QString& label) const
{
    QVariantMap out;
    const auto s = _core.snapshot(label.toStdString());
    out.insert(QStringLiteral("label"), QString::fromStdString(s.label));
    out.insert(QStringLiteral("tMs"),   static_cast<qint64>(s.t_ms));
    QVariantMap values;
    for (const auto& kv : s.values) {
        values.insert(QString::fromStdString(kv.first), kv.second);
    }
    out.insert(QStringLiteral("values"), values);
    return out;
}

int TuningController::setParameter(const QString& name, double value)
{
    double applied = 0.0;
    const auto rc = _core.set(name.toStdString(), value, &applied);
    switch (rc) {
    case SetResult::Ok:
        _last_error.clear();
        _model->rebuild();
        emit parameterChanged(name, applied);
        emit lastErrorChanged();
        break;
    default:
        _last_error = QString::fromLatin1(tuning::describe(rc));
        emit lastErrorChanged();
        break;
    }
    return static_cast<int>(rc);
}

void TuningController::resetToDefaults()
{
    _core.resetToDefaults();
    _model->rebuild();
    emit snapshotTaken(QStringLiteral("reset"), static_cast<qint64>(nowMsEpoch()));
}

bool TuningController::_safetyGate(const std::string& /*name*/, double /*cur*/, double /*req*/) const
{
    if (!phaseAllowsTuning(_phase)) return false;
    if (!_step_up_fresh)            return false;
    return true;
}

} // namespace m130::gui

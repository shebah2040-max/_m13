#include "MissionController.h"

#include <QtCore/QDateTime>

#include <chrono>
#include <utility>

namespace m130::gui {

namespace {
quint64 wallMsNow()
{
    using namespace std::chrono;
    return static_cast<quint64>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}
quint64 wallUsNow()
{
    using namespace std::chrono;
    return static_cast<quint64>(
        duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
}
} // namespace

// ---------------- ChecklistListModel ----------------

ChecklistListModel::ChecklistListModel(const views::ChecklistModel* src, QObject* parent)
    : QAbstractListModel(parent), _src(src)
{
}

void ChecklistListModel::refresh()
{
    beginResetModel();
    endResetModel();
}

int ChecklistListModel::rowCount(const QModelIndex&) const
{
    return _src ? static_cast<int>(_src->size()) : 0;
}

QVariant ChecklistListModel::data(const QModelIndex& index, int role) const
{
    if (!_src || !index.isValid() || index.row() < 0 ||
        index.row() >= static_cast<int>(_src->size())) {
        return {};
    }
    const auto& item = _src->items().at(static_cast<std::size_t>(index.row()));
    switch (role) {
    case IdRole:           return QString::fromStdString(item.id);
    case TitleRole:        return QString::fromStdString(item.title);
    case StatusRole:       return static_cast<int>(item.status);
    case RequiresAuthRole: return item.requires_auth;
    case RequiredRoleRole: return static_cast<int>(item.required_role);
    case CompletedByRole:  return QString::fromStdString(item.completed_by);
    case NotesRole:        return QString::fromStdString(item.notes);
    }
    return {};
}

QHash<int, QByteArray> ChecklistListModel::roleNames() const
{
    return {
        {IdRole,           "itemId"},
        {TitleRole,        "title"},
        {StatusRole,       "status"},
        {RequiresAuthRole, "requiresAuth"},
        {RequiredRoleRole, "requiredRole"},
        {CompletedByRole,  "completedBy"},
        {NotesRole,        "notes"},
    };
}

// ---------------- ReplayControllerQml ----------------

ReplayControllerQml::ReplayControllerQml(QObject* parent) : QObject(parent) {}

int ReplayControllerQml::stateInt() const { return static_cast<int>(_rc.state()); }
QString ReplayControllerQml::path() const { return QString::fromStdString(_rc.path()); }

bool ReplayControllerQml::loadFile(const QString& path)
{
    const bool ok = _rc.loadFile(path.toStdString());
    emit fileChanged();
    emit stateChanged();
    return ok;
}

void ReplayControllerQml::play()
{
    _rc.play(wallUsNow());
    emit stateChanged();
}

void ReplayControllerQml::pause()
{
    _rc.pause();
    emit stateChanged();
}

void ReplayControllerQml::setSpeed(float speed)
{
    _rc.setSpeed(speed);
    emit speedChanged();
}

bool ReplayControllerQml::rewind()
{
    const bool ok = _rc.rewind();
    if (ok) emit stateChanged();
    return ok;
}

// ---------------- MissionController ----------------

quint64 MissionController::_nowMs() { return wallMsNow(); }

MissionController::MissionController(QObject* parent)
    : QObject(parent)
    , _checklist_model(new ChecklistListModel(&_checklist, this))
    , _replay_qml(new ReplayControllerQml(this))
{
    _tick.setInterval(100);
    connect(&_tick, &QTimer::timeout, this, &MissionController::_onTick);
    _tick.start();
}

MissionController::~MissionController() = default;

int     MissionController::countdownStateInt()      const { return static_cast<int>(_countdown.state()); }
double  MissionController::countdownSecondsLeft()   const { return _countdown.secondsToLaunch(_nowMs()); }
QString MissionController::countdownLabel()         const { return QString::fromStdString(_countdown.label(_nowMs())); }
quint64 MissionController::countdownHoldElapsedMs() const noexcept { return _countdown.holdElapsedMs(); }
bool    MissionController::readyForLaunch()         const { return _checklist.isReadyForLaunch(); }

void MissionController::installDefaultChecklist()
{
    if (!_checklist.empty()) return;
    auto add = [&](std::string id, std::string title, bool auth, access::Role role) {
        views::ChecklistItem it;
        it.id = std::move(id);
        it.title = std::move(title);
        it.requires_auth = auth;
        it.required_role = role;
        _checklist.add(it);
    };
    add("range.clear",        "Range declared clear (RSO)",     true,  access::Role::RangeSafety);
    add("weather.go",         "Weather go/no-go",                true,  access::Role::SafetyOfficer);
    add("fts.self_test",      "FTS self-test PASS",              true,  access::Role::SafetyOfficer);
    add("propellant.loaded",  "Propellant loaded + vented",      true,  access::Role::Operator);
    add("avionics.power",     "Avionics power nominal",          false, access::Role::Observer);
    add("telemetry.open",     "Telemetry link open (<1s latency)", false, access::Role::Observer);
    add("gps.fix3d",          "GPS 3D fix (>8 sats)",            false, access::Role::Observer);
    add("mission.request_arm","Flight Director requests ARM",    true,  access::Role::FlightDirector);
    _checklist_model->refresh();
    emit checklistChanged();
}

bool MissionController::armCountdown(qint64 targetLaunchMsSinceEpoch)
{
    if (targetLaunchMsSinceEpoch < 0) return false;
    const bool ok = _countdown.arm(static_cast<std::uint64_t>(targetLaunchMsSinceEpoch), _nowMs());
    emit countdownChanged();
    return ok;
}

bool MissionController::holdCountdown()
{
    const bool ok = _countdown.hold(_nowMs());
    emit countdownChanged();
    return ok;
}

bool MissionController::resumeCountdown()
{
    const bool ok = _countdown.resume(_nowMs());
    emit countdownChanged();
    return ok;
}

void MissionController::abortCountdown()
{
    _countdown.abort(_nowMs());
    emit countdownChanged();
}

bool MissionController::markChecklistDone(const QString& id, const QString& user,
                                          int role, const QString& notes)
{
    const bool ok = _checklist.markDone(id.toStdString(), user.toStdString(),
                                        static_cast<access::Role>(role),
                                        notes.toStdString());
    _checklist_model->refresh();
    emit checklistChanged();
    return ok;
}

bool MissionController::skipChecklistItem(const QString& id, const QString& user,
                                          const QString& reason)
{
    const bool ok = _checklist.skip(id.toStdString(), user.toStdString(),
                                    reason.toStdString());
    _checklist_model->refresh();
    emit checklistChanged();
    return ok;
}

bool MissionController::blockChecklistItem(const QString& id, const QString& reason)
{
    const bool ok = _checklist.block(id.toStdString(), reason.toStdString());
    _checklist_model->refresh();
    emit checklistChanged();
    return ok;
}

void MissionController::resetChecklist()
{
    _checklist.reset();
    _checklist_model->refresh();
    emit checklistChanged();
}

void MissionController::updateIip(double pos_down_m, double pos_cross_m, double pos_up_m,
                                  double vel_down_m_s, double vel_cross_m_s, double vel_up_m_s)
{
    _iip = views::IipPredictor::predict(pos_down_m, pos_cross_m, pos_up_m,
                                        vel_down_m_s, vel_cross_m_s, vel_up_m_s);
    emit iipChanged();
}

void MissionController::_onTick()
{
    _countdown.tick(_nowMs());
    emit countdownChanged();
    if (_replay_qml->_rc.isPlaying()) {
        const std::size_t emitted = _replay_qml->_rc.step(wallUsNow());
        if (emitted > 0) {
            emit _replay_qml->frameAdvanced();
        }
        if (_replay_qml->_rc.isFinished()) {
            emit _replay_qml->stateChanged();
        }
    }
}

} // namespace m130::gui

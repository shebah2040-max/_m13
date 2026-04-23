#include "SafetyKernel.h"

#include <QtCore/QDebug>
#include <QtCore/QVector>

using m130::safety::Alert;
using m130::safety::AlertLevel;
using m130::safety::FlightPhase;
using m130::safety::SafetyAggregator;
using m130::safety::TransitionRecord;
using m130::safety::TransitionResult;

namespace m130::gui {

SafetyKernel::SafetyKernel(QObject* parent)
    : QObject(parent)
    , _model(new AlertListModel(this))
{
    wire();
    _tick_timer.setInterval(_tick_interval_ms);
    _tick_timer.setTimerType(Qt::PreciseTimer);
    connect(&_tick_timer, &QTimer::timeout, this, &SafetyKernel::onTick);
}

SafetyKernel::~SafetyKernel() = default;

void SafetyKernel::installDefaults()
{
    _agg.installDefaults();
    _agg.addChannel("heartbeat");
    _agg.addChannel("gnc_state");
    _agg.addChannel("mhe");
    _agg.addChannel("mpc");

    // Move off Unknown → Idle so command authorisations have a real phase.
    _agg.requestTransition(FlightPhase::Idle, "boot");

    if (!_tick_timer.isActive()) {
        _tick_timer.start();
    }
}

void SafetyKernel::wire()
{
    _agg.subscribeAlerts([this](const Alert& a, bool is_ack) {
        handleAlert(a, is_ack);
    });
    _agg.subscribeMission([this](const TransitionRecord& r) {
        handleTransition(r);
    });
}

void SafetyKernel::handleAlert(const Alert& a, bool is_ack)
{
    QVector<Alert> snapshot;
    for (const auto& x : _agg.alerts().active()) {
        snapshot.push_back(x);
    }
    _model->refresh(std::move(snapshot));

    const AlertLevel level_now = _agg.masterLevel();
    if (level_now != _last_master) {
        _last_master = level_now;
        emit masterLevelChanged();
    }
    emit alertsChanged();

    if (is_ack) {
        emit alertAcknowledged(QString::fromStdString(a.id),
                               QString::fromStdString(a.ack_user));
    } else {
        emit alertRaised(QString::fromStdString(a.id),
                         int(a.level),
                         QString::fromStdString(a.title));
    }
}

void SafetyKernel::handleTransition(const TransitionRecord& r)
{
    if (r.result != TransitionResult::Accepted) return;
    if (r.to == _last_phase) return;
    _last_phase = r.to;
    emit phaseChanged();
}

void SafetyKernel::onTick()
{
    _agg.tick();
}

int SafetyKernel::currentPhaseInt() const
{
    return int(_agg.currentPhase());
}

QString SafetyKernel::currentPhaseText() const
{
    return QString::fromUtf8(safety::toString(_agg.currentPhase()).data());
}

int SafetyKernel::masterLevelInt() const
{
    return int(_agg.masterLevel());
}

int SafetyKernel::activeAlertCount() const
{
    return int(_agg.activeAlertCount());
}

quint64 SafetyKernel::totalAlertsRaised() const
{
    return static_cast<quint64>(_agg.alerts().raisedCount());
}

void SafetyKernel::setTickIntervalMs(int ms)
{
    if (ms < 10) ms = 10;
    if (ms == _tick_interval_ms) return;
    _tick_interval_ms = ms;
    _tick_timer.setInterval(ms);
    emit tickIntervalMsChanged();
}

void SafetyKernel::feed(const QString& channel)
{
    _agg.feed(channel.toStdString());
}

int SafetyKernel::evaluateSample(const QString& variable, double value)
{
    const auto r = _agg.evaluateSample(variable.toStdString(), value);
    return int(r.level);
}

int SafetyKernel::requestTransition(int phase, const QString& reason)
{
    if (phase < int(FlightPhase::Unknown) || phase > int(FlightPhase::Abort)) {
        return int(TransitionResult::RejectedIllegal);
    }
    return int(_agg.requestTransition(FlightPhase(phase), reason.toStdString()));
}

bool SafetyKernel::acknowledge(const QString& id, const QString& user)
{
    return _agg.alerts().acknowledge(id.toStdString(), user.toStdString());
}

} // namespace m130::gui

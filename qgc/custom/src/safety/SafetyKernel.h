#pragma once

#include "AlertLevel.h"
#include "AlertListModel.h"
#include "FlightPhase.h"
#include "SafetyAggregator.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <memory>

namespace m130::gui {

/// Qt-bound facade for the pure-C++ SafetyAggregator.
///
/// Owns the aggregator plus a periodic watchdog tick timer, and re-exports
/// state as Qt properties / signals so QML components (MasterCautionLight,
/// AlertBanner, mission banners, command panels) can bind directly.
///
/// ``Q_INVOKABLE`` methods mirror the aggregator's input surface so the
/// plugin code (CustomPlugin::mavlinkMessage) and QML alike can drive it.
class SafetyKernel : public QObject
{
    Q_OBJECT

    /// Current flight phase (int corresponds to m130::safety::FlightPhase).
    Q_PROPERTY(int currentPhase READ currentPhaseInt NOTIFY phaseChanged)
    /// Human-readable phase string ("IDLE", "PRELAUNCH", ...).
    Q_PROPERTY(QString currentPhaseText READ currentPhaseText NOTIFY phaseChanged)
    /// Highest unacknowledged alert level (0=None … 4=Emergency).
    Q_PROPERTY(int masterLevel READ masterLevelInt NOTIFY masterLevelChanged)
    /// Number of active unacked alerts.
    Q_PROPERTY(int activeAlertCount READ activeAlertCount NOTIFY alertsChanged)
    /// List model backing QML list views.
    Q_PROPERTY(AlertListModel* activeAlerts READ activeAlerts CONSTANT)
    /// Running total of alerts raised since construction.
    Q_PROPERTY(quint64 totalAlertsRaised READ totalAlertsRaised NOTIFY alertsChanged)
    /// Tick interval in milliseconds (watchdog sweep cadence).
    Q_PROPERTY(int tickIntervalMs READ tickIntervalMs WRITE setTickIntervalMs
                   NOTIFY tickIntervalMsChanged)

public:
    explicit SafetyKernel(QObject* parent = nullptr);
    ~SafetyKernel() override;

    /// Load the default envelope + RBAC policy and the standard channel set
    /// (heartbeat, gnc_state, mhe, mpc). Call once after construction.
    Q_INVOKABLE void installDefaults();

    // ─── Property getters ──────────────────────────────────────────────
    int currentPhaseInt() const;
    QString currentPhaseText() const;
    int masterLevelInt() const;
    int activeAlertCount() const;
    AlertListModel* activeAlerts() const { return _model; }
    quint64 totalAlertsRaised() const;
    int tickIntervalMs() const { return _tick_interval_ms; }
    void setTickIntervalMs(int ms);

    // ─── Input ops (typically driven from CustomPlugin) ────────────────
    Q_INVOKABLE void feed(const QString& channel);
    Q_INVOKABLE int  evaluateSample(const QString& variable, double value);

    /// Request a mission phase transition; returns the TransitionResult int.
    Q_INVOKABLE int requestTransition(int phase, const QString& reason = {});

    /// Acknowledge an alert from QML.
    Q_INVOKABLE bool acknowledge(const QString& id, const QString& user);

    /// Direct accessor for non-QML integration (CustomPlugin, tests).
    safety::SafetyAggregator& aggregator() noexcept { return _agg; }
    const safety::SafetyAggregator& aggregator() const noexcept { return _agg; }

signals:
    void phaseChanged();
    void masterLevelChanged();
    /// Emitted when an alert is raised, escalated, or acknowledged.
    void alertsChanged();
    /// Fired once per raise event with id+level for transient UI feedback.
    void alertRaised(const QString& id, int level, const QString& title);
    void alertAcknowledged(const QString& id, const QString& user);
    void tickIntervalMsChanged();

private:
    void wire();
    void onTick();
    void handleAlert(const safety::Alert& a, bool is_ack);
    void handleTransition(const safety::TransitionRecord& r);

    safety::SafetyAggregator _agg;
    AlertListModel*          _model;
    QTimer                   _tick_timer;
    int                      _tick_interval_ms = 200; ///< 5 Hz default
    safety::AlertLevel       _last_master = safety::AlertLevel::None;
    safety::FlightPhase      _last_phase = safety::FlightPhase::Unknown;
};

} // namespace m130::gui

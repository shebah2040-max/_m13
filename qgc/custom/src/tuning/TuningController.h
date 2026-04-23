#pragma once

#include "MpcTuningModel.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>

#include <memory>
#include <string>
#include <vector>

namespace m130::access { class SessionManager; }

namespace m130::gui {

class TuningParamModel;

/// Qt-bound facade for `tuning::MpcTuningModel`.
///
/// The safety-gate predicate is wired to three inputs:
///   * `allowedPhases` — a phase bit-mask from the mission state machine
///     (bit 0 = Idle, bit 1 = PreLaunch, …). Defaults to `Idle | PreLaunch`.
///   * `sessionProvider` — a callback returning the current session id,
///     consulted when `requireStepUp == true` to verify freshness via the
///     owning SessionManager.
///   * `missionPhaseProvider` — a callback returning the current phase.
class TuningController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(m130::gui::TuningParamModel* parameters READ parameters CONSTANT)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int currentPhase READ currentPhase WRITE setCurrentPhase NOTIFY currentPhaseChanged)
    Q_PROPERTY(bool stepUpFresh READ stepUpFresh WRITE setStepUpFresh NOTIFY stepUpFreshChanged)

public:
    explicit TuningController(QObject* parent = nullptr);
    ~TuningController() override;

    TuningParamModel* parameters() const noexcept { return _model; }
    QString lastError() const noexcept { return _last_error; }
    int  currentPhase() const noexcept { return _phase; }
    bool stepUpFresh()  const noexcept { return _step_up_fresh; }

    void setCurrentPhase(int phase);
    void setStepUpFresh(bool fresh);

    tuning::MpcTuningModel& model() noexcept { return _core; }

    Q_INVOKABLE void installDefaults();
    Q_INVOKABLE QVariantMap snapshotToVariant(const QString& label = {}) const;

public slots:
    /// Returns the `tuning::SetResult` enum as int (0 = Ok). On success
    /// emits `parameterChanged(name, newValue)`.
    int setParameter(const QString& name, double value);

    void resetToDefaults();

signals:
    void parameterChanged(const QString& name, double value);
    void lastErrorChanged();
    void currentPhaseChanged();
    void stepUpFreshChanged();
    void snapshotTaken(const QString& label, qint64 tMs);

private:
    bool _safetyGate(const std::string& name, double cur, double req) const;

    tuning::MpcTuningModel _core;
    TuningParamModel*      _model;
    int                    _phase = 0;            ///< FlightPhase int (default Idle)
    bool                   _step_up_fresh = false;
    QString                _last_error;
};

/// QAbstractListModel over the parameter list. Refreshed on every successful
/// set; read-only from QML's point of view — use `TuningController::setParameter`.
class TuningParamModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ValueRole,
        MinRole,
        MaxRole,
        DefaultRole,
        UnitRole,
        DescriptionRole,
    };

    explicit TuningParamModel(const tuning::MpcTuningModel* core,
                              QObject* parent = nullptr);

    void rebuild();

    int               rowCount(const QModelIndex& parent = {}) const override;
    QVariant          data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    const tuning::MpcTuningModel* _core;
    std::vector<std::string>      _names;   ///< Snapshot of ordered names
};

} // namespace m130::gui

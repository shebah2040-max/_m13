#pragma once

#include "ChecklistModel.h"
#include "CountdownClock.h"
#include "IipPredictor.h"
#include "ReplayController.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVector>

namespace m130::gui {

class ChecklistListModel;
class ReplayControllerQml;

/// Qt-bound facade for the Pillar 5 view-models (pre-launch checklist,
/// T-X countdown, IIP predictor, FDR replay controller).
///
/// All heavy work stays in the pure-C++ cores (`views::*`); this class only
/// wires Q_PROPERTY / NOTIFY signals and drives a periodic `QTimer` to tick
/// the countdown at 10 Hz so QML bindings refresh smoothly.
class MissionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(m130::gui::ChecklistListModel* checklist READ checklist CONSTANT)
    Q_PROPERTY(m130::gui::ReplayControllerQml* replay  READ replay  CONSTANT)

    Q_PROPERTY(int      countdownState        READ countdownStateInt NOTIFY countdownChanged)
    Q_PROPERTY(double   countdownSecondsLeft  READ countdownSecondsLeft NOTIFY countdownChanged)
    Q_PROPERTY(QString  countdownLabel        READ countdownLabel    NOTIFY countdownChanged)
    Q_PROPERTY(quint64  countdownHoldElapsedMs READ countdownHoldElapsedMs NOTIFY countdownChanged)
    Q_PROPERTY(bool     readyForLaunch        READ readyForLaunch    NOTIFY checklistChanged)

    Q_PROPERTY(double   iipDownrangeM         READ iipDownrangeM  NOTIFY iipChanged)
    Q_PROPERTY(double   iipCrossrangeM        READ iipCrossrangeM NOTIFY iipChanged)
    Q_PROPERTY(double   iipTimeToImpactS      READ iipTimeToImpactS NOTIFY iipChanged)
    Q_PROPERTY(bool     iipValid              READ iipValid       NOTIFY iipChanged)

public:
    explicit MissionController(QObject* parent = nullptr);
    ~MissionController() override;

    ChecklistListModel*  checklist() const noexcept { return _checklist_model; }
    ReplayControllerQml* replay()    const noexcept { return _replay_qml; }

    int      countdownStateInt()        const;
    double   countdownSecondsLeft()     const;
    QString  countdownLabel()           const;
    quint64  countdownHoldElapsedMs()   const noexcept;
    bool     readyForLaunch()           const;

    double   iipDownrangeM()   const noexcept { return _iip.impact_downrange_m; }
    double   iipCrossrangeM()  const noexcept { return _iip.impact_crossrange_m; }
    double   iipTimeToImpactS()const noexcept { return _iip.time_to_impact_s; }
    bool     iipValid()        const noexcept { return _iip.valid; }

    /// Install the standard M130 pre-launch checklist. Safe to call once.
    Q_INVOKABLE void installDefaultChecklist();

public slots:
    bool armCountdown(qint64 targetLaunchMsSinceEpoch);
    bool holdCountdown();
    bool resumeCountdown();
    void abortCountdown();

    bool markChecklistDone(const QString& id, const QString& user, int role,
                           const QString& notes = {});
    bool skipChecklistItem(const QString& id, const QString& user,
                           const QString& reason);
    bool blockChecklistItem(const QString& id, const QString& reason);
    void resetChecklist();

    /// Update the IIP prediction from the current GNC state. Units per
    /// `views::IipPredictor` — downrange/crossrange/up metres, velocities
    /// m/s, up positive.
    void updateIip(double pos_down_m, double pos_cross_m, double pos_up_m,
                   double vel_down_m_s, double vel_cross_m_s, double vel_up_m_s);

signals:
    void countdownChanged();
    void checklistChanged();
    void iipChanged();

private:
    void _onTick();
    static quint64 _nowMs();

    views::ChecklistModel    _checklist;
    views::CountdownClock    _countdown;
    views::IipResult         _iip;
    ChecklistListModel*      _checklist_model;
    ReplayControllerQml*     _replay_qml;
    QTimer                   _tick;
};

/// QAbstractListModel adapter over `views::ChecklistModel`. Rebuilt on every
/// mutation — the underlying list is small (≤ 30 items) so a full reset is
/// cheaper than incremental diff tracking.
class ChecklistListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole        = Qt::UserRole + 1,
        TitleRole,
        StatusRole,
        RequiresAuthRole,
        RequiredRoleRole,
        CompletedByRole,
        NotesRole,
    };

    explicit ChecklistListModel(const views::ChecklistModel* src,
                                QObject* parent = nullptr);

    void refresh();

    int               rowCount(const QModelIndex& parent = {}) const override;
    QVariant          data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    const views::ChecklistModel* _src;
};

/// Minimal QObject facade for `views::ReplayController`. Engine ticks are
/// currently driven by the owning MissionController's timer when playback
/// is active; this keeps the replay subsystem mostly-Qt-free.
class ReplayControllerQml : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int     state         READ stateInt     NOTIFY stateChanged)
    Q_PROPERTY(float   speed         READ speed        NOTIFY speedChanged)
    Q_PROPERTY(quint64 framesEmitted READ framesEmitted NOTIFY frameAdvanced)
    Q_PROPERTY(QString path          READ path         NOTIFY fileChanged)
public:
    explicit ReplayControllerQml(QObject* parent = nullptr);

    int     stateInt()      const;
    float   speed()         const noexcept { return _rc.speed(); }
    quint64 framesEmitted() const noexcept { return static_cast<quint64>(_rc.framesEmitted()); }
    QString path()          const;

public slots:
    bool loadFile(const QString& path);
    void play();
    void pause();
    void setSpeed(float speed);
    bool rewind();

signals:
    void stateChanged();
    void speedChanged();
    void fileChanged();
    void frameAdvanced();

private:
    views::ReplayController _rc;
    friend class MissionController;
};

} // namespace m130::gui

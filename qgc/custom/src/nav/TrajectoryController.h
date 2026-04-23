#pragma once

#include "TrajectoryModel.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVariantList>

namespace m130::gui {

class TrajectoryTrackModel;

/// Qt-bound facade for `nav::TrajectoryModel`.
///
/// Accepts GNC samples from `CustomPlugin::mavlinkMessage()` and exposes
/// projection-friendly ENU arrays to QML Canvas views. Rendering choices
/// (Cesium, Qt Quick 3D, Canvas 2D) are the view's responsibility.
class TrajectoryController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool     hasOrigin    READ hasOrigin    NOTIFY originChanged)
    Q_PROPERTY(double   originLat    READ originLat    NOTIFY originChanged)
    Q_PROPERTY(double   originLon    READ originLon    NOTIFY originChanged)
    Q_PROPERTY(int      sampleCount  READ sampleCount  NOTIFY trackChanged)
    Q_PROPERTY(m130::gui::TrajectoryTrackModel* track READ track CONSTANT)

public:
    explicit TrajectoryController(QObject* parent = nullptr);
    ~TrajectoryController() override;

    nav::TrajectoryModel& model() noexcept { return _core; }
    TrajectoryTrackModel* track() const noexcept { return _track_model; }

    bool    hasOrigin() const noexcept { return _core.hasOrigin(); }
    double  originLat() const noexcept { return _core.origin().lat_deg; }
    double  originLon() const noexcept { return _core.origin().lon_deg; }
    int     sampleCount() const noexcept
    { return static_cast<int>(_core.samples().size()); }

public slots:
    void setOrigin(double lat_deg, double lon_deg, double alt_m);
    void clear();

    /// Append a live GNC sample (milliseconds since epoch). Returns true
    /// when the sample was accepted.
    bool appendSample(qint64 t_ms,
                      double lat_deg, double lon_deg, double alt_m,
                      double ve_mps, double vn_mps, double vu_mps);

    /// Append a predicted IIP at system time @p t_ms.
    void appendIip(qint64 t_ms, double lat_deg, double lon_deg, double alt_m);

signals:
    void originChanged();
    void trackChanged();

private:
    nav::TrajectoryModel  _core;
    TrajectoryTrackModel* _track_model;
};

/// QAbstractListModel exposing downsampled ENU track points to QML.
/// Row 0 is the oldest retained sample; last row is the latest.
class TrajectoryTrackModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        EastRole  = Qt::UserRole + 1,
        NorthRole,
        UpRole,
    };

    explicit TrajectoryTrackModel(nav::TrajectoryModel* core, QObject* parent = nullptr);

    void rebuild();

    int                    rowCount(const QModelIndex& parent = {}) const override;
    QVariant               data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    nav::TrajectoryModel*        _core;
    std::vector<geo::EnuOffset>  _points;
};

} // namespace m130::gui

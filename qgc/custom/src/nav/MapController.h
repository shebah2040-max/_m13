#pragma once

#include "CorridorMonitor.h"
#include "MapCorridor.h"

#include <QtCore/QObject>
#include <QtCore/QVariantList>

namespace m130::gui {

/// Qt-bound facade for `nav::MapCorridor` + `nav::CorridorMonitor`.
///
/// Corridor geometry is configured from QML/JSON via Q_INVOKABLE setters.
/// Live telemetry calls `updatePosition()`; the controller emits state
/// transitions so range safety can surface them to the AlertManager.
class MapController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int     state           READ state           NOTIFY stateChanged)
    Q_PROPERTY(double  marginMeters    READ marginMeters    NOTIFY stateChanged)
    Q_PROPERTY(double  warnMarginMeters
               READ warnMarginMeters WRITE setWarnMarginMeters NOTIFY configChanged)
    Q_PROPERTY(double  clearMarginMeters
               READ clearMarginMeters WRITE setClearMarginMeters NOTIFY configChanged)
    Q_PROPERTY(int     breachPersistenceMs
               READ breachPersistenceMs WRITE setBreachPersistenceMs NOTIFY configChanged)

public:
    explicit MapController(QObject* parent = nullptr);
    ~MapController() override;

    nav::MapCorridor&     corridor() noexcept { return _corridor; }
    nav::CorridorMonitor& monitor()  noexcept { return _monitor; }

    int    state()               const noexcept;
    double marginMeters()        const noexcept { return _last_margin; }
    double warnMarginMeters()    const noexcept { return _monitor.config().warn_margin_m; }
    double clearMarginMeters()   const noexcept { return _monitor.config().clear_margin_m; }
    int    breachPersistenceMs() const noexcept;

    void setWarnMarginMeters(double v);
    void setClearMarginMeters(double v);
    void setBreachPersistenceMs(int v);

public slots:
    /// Replace the polygon. `vertices` is a list of `QPointF(lat, lon)`.
    void setPolygon(const QVariantList& vertices);

    /// Add a single centreline leg.
    void addTrackLeg(double lat_a, double lon_a,
                     double lat_b, double lon_b, double half_width_m);

    void clearCorridor();

    /// Update monitor with a live position.
    void updatePosition(double lat_deg, double lon_deg, double alt_m);

signals:
    void stateChanged();
    void configChanged();
    void corridorBreached();
    void corridorCleared();

private:
    nav::MapCorridor     _corridor;
    nav::CorridorMonitor _monitor;
    double               _last_margin = 0.0;
};

} // namespace m130::gui

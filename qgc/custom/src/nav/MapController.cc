#include "MapController.h"

#include <QtCore/QPointF>
#include <chrono>

namespace m130::gui {

MapController::MapController(QObject* parent) : QObject(parent)
{
    _monitor.setCorridor(&_corridor);
}

MapController::~MapController() = default;

int MapController::state() const noexcept
{
    return static_cast<int>(_monitor.state());
}

int MapController::breachPersistenceMs() const noexcept
{
    return static_cast<int>(_monitor.config().breach_persistence.count());
}

void MapController::setWarnMarginMeters(double v)
{
    auto cfg = _monitor.config();
    cfg.warn_margin_m = v;
    _monitor.setConfig(cfg);
    emit configChanged();
}

void MapController::setClearMarginMeters(double v)
{
    auto cfg = _monitor.config();
    cfg.clear_margin_m = v;
    _monitor.setConfig(cfg);
    emit configChanged();
}

void MapController::setBreachPersistenceMs(int v)
{
    auto cfg = _monitor.config();
    cfg.breach_persistence = std::chrono::milliseconds(v);
    _monitor.setConfig(cfg);
    emit configChanged();
}

void MapController::setPolygon(const QVariantList& vertices)
{
    std::vector<geo::GeoPoint> poly;
    poly.reserve(vertices.size());
    for (const auto& v : vertices) {
        const QPointF p = v.toPointF();
        poly.push_back(geo::GeoPoint{p.x(), p.y(), 0.0});
    }
    _corridor.setPolygon(std::move(poly));
    emit stateChanged();
}

void MapController::addTrackLeg(double lat_a, double lon_a,
                                double lat_b, double lon_b, double half_width_m)
{
    _corridor.addTrackLeg({lat_a, lon_a, 0.0}, {lat_b, lon_b, 0.0}, half_width_m);
    emit stateChanged();
}

void MapController::clearCorridor()
{
    _corridor.clear();
    emit stateChanged();
}

void MapController::updatePosition(double lat_deg, double lon_deg, double alt_m)
{
    const auto s = _monitor.update({lat_deg, lon_deg, alt_m}, std::chrono::steady_clock::now());
    _last_margin = s.margin;
    emit stateChanged();
    if (s.just_breached) emit corridorBreached();
    if (s.just_cleared)  emit corridorCleared();
}

} // namespace m130::gui

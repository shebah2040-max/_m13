#include "MapCorridor.h"

#include "../geo/Geodesic.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace m130::nav {

namespace {

/// Minimum perpendicular distance [m] from @p to any edge of a
/// polygon in lon/lat degrees. Treated as locally Euclidean because
/// corridor breach margins are consulted close to the boundary — good
/// enough for UI warnings, not for course plotting.
double polygonPerimeterDistance(const std::vector<geo::GeoPoint>& poly,
                                const geo::GeoPoint& p)
{
    double best = std::numeric_limits<double>::infinity();
    const std::size_t n = poly.size();
    for (std::size_t i = 0; i < n; ++i) {
        const auto& a = poly[i];
        const auto& b = poly[(i + 1) % n];
        const double d = std::abs(geo::crossTrackDistance(a, b, p));
        if (d < best) best = d;
    }
    return best;
}

bool rayCastInside(const std::vector<geo::GeoPoint>& poly, const geo::GeoPoint& p)
{
    bool inside = false;
    const std::size_t n = poly.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const double xi = poly[i].lon_deg;
        const double yi = poly[i].lat_deg;
        const double xj = poly[j].lon_deg;
        const double yj = poly[j].lat_deg;
        const bool crosses = ((yi > p.lat_deg) != (yj > p.lat_deg))
            && (p.lon_deg < (xj - xi) * (p.lat_deg - yi) / (yj - yi) + xi);
        if (crosses) inside = !inside;
    }
    return inside;
}

} // namespace

void MapCorridor::setPolygon(std::vector<geo::GeoPoint> vertices)
{
    _polygon = std::move(vertices);
}

void MapCorridor::addTrackLeg(const geo::GeoPoint& a, const geo::GeoPoint& b, double half_width_m)
{
    if (half_width_m <= 0.0) return;
    _legs.push_back({a, b, half_width_m});
}

void MapCorridor::clear()
{
    _polygon.clear();
    _legs.clear();
}

bool MapCorridor::_polygonContains(const geo::GeoPoint& p) const
{
    if (_polygon.size() < 3) return true;
    return rayCastInside(_polygon, p);
}

bool MapCorridor::contains(const geo::GeoPoint& p) const
{
    if (_polygon.empty() && _legs.empty()) return false;
    if (!_polygon.empty() && !_polygonContains(p)) return false;

    for (const auto& leg : _legs) {
        const double d = std::abs(geo::crossTrackDistance(leg.a, leg.b, p));
        if (d > leg.half_width_m) return false;
    }
    return true;
}

std::optional<double> MapCorridor::marginMeters(const geo::GeoPoint& p) const
{
    if (_polygon.empty() && _legs.empty()) return std::nullopt;

    double margin = std::numeric_limits<double>::infinity();

    if (!_polygon.empty()) {
        const bool inside = _polygonContains(p);
        const double d = polygonPerimeterDistance(_polygon, p);
        margin = std::min(margin, inside ? d : -d);
    }
    for (const auto& leg : _legs) {
        const double d = std::abs(geo::crossTrackDistance(leg.a, leg.b, p));
        margin = std::min(margin, leg.half_width_m - d);
    }
    return margin;
}

} // namespace m130::nav

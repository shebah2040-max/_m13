#include "NotamModel.h"

#include "../geo/Geodesic.h"

#include <algorithm>

namespace m130::weather {

namespace {

bool polygonContains(const std::vector<geo::GeoPoint>& poly, const geo::GeoPoint& p)
{
    if (poly.size() < 3) return false;
    bool inside = false;
    const std::size_t n = poly.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const double xi = poly[i].lon_deg;
        const double yi = poly[i].lat_deg;
        const double xj = poly[j].lon_deg;
        const double yj = poly[j].lat_deg;
        if (((yi > p.lat_deg) != (yj > p.lat_deg))
            && (p.lon_deg < (xj - xi) * (p.lat_deg - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace

bool NotamArea::contains(const geo::GeoPoint& p) const
{
    if (isPolygon()) return polygonContains(polygon, p);
    if (isCircle())  return geo::haversineDistance(centre, p) <= radius_m;
    return false;
}

std::vector<const Notam*>
NotamModel::activeAt(std::chrono::system_clock::time_point when) const
{
    std::vector<const Notam*> out;
    for (const auto& n : _items) {
        if (n.activeAt(when)) out.push_back(&n);
    }
    return out;
}

NotamSeverity NotamModel::worstAt(const geo::GeoPoint& p,
                                  std::chrono::system_clock::time_point when) const
{
    NotamSeverity worst = NotamSeverity::Info;
    for (const auto& n : _items) {
        if (!n.activeAt(when)) continue;
        if (!n.area.contains(p)) continue;
        if (static_cast<std::uint8_t>(n.severity) > static_cast<std::uint8_t>(worst)) {
            worst = n.severity;
        }
    }
    return worst;
}

bool NotamModel::isAffected(const geo::GeoPoint& p,
                            std::chrono::system_clock::time_point when) const
{
    for (const auto& n : _items) {
        if (n.activeAt(when) && n.area.contains(p)) return true;
    }
    return false;
}

} // namespace m130::weather

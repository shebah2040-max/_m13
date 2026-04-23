#pragma once

#include "../geo/Wgs84.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace m130::weather {

enum class NotamSeverity : std::uint8_t {
    Info     = 0,
    Advisory = 1,
    Warning  = 2,
    Hazard   = 3,
};

/// Area of interest for a NOTAM. Either a polygon or a bounding circle
/// is populated.
struct NotamArea {
    std::vector<geo::GeoPoint> polygon;   ///< either/or
    geo::GeoPoint              centre;
    double                     radius_m = 0.0;

    bool isPolygon() const noexcept { return !polygon.empty(); }
    bool isCircle()  const noexcept { return radius_m > 0.0 && polygon.empty(); }
    bool contains(const geo::GeoPoint& p) const;
};

struct Notam {
    std::string   id;
    std::string   summary;
    NotamSeverity severity = NotamSeverity::Info;
    std::chrono::system_clock::time_point start{};
    std::chrono::system_clock::time_point end{};
    NotamArea     area;

    bool activeAt(std::chrono::system_clock::time_point when) const noexcept
    { return when >= start && when <= end; }
};

/// In-memory NOTAM index. Append-only and queryable by time/point.
class NotamModel
{
public:
    void clear() { _items.clear(); }
    void add(Notam n) { _items.push_back(std::move(n)); }
    std::size_t size() const noexcept { return _items.size(); }

    const std::vector<Notam>& items() const noexcept { return _items; }

    /// All NOTAMs active at @p when.
    std::vector<const Notam*> activeAt(std::chrono::system_clock::time_point when) const;

    /// Highest severity of any NOTAM that is both active at @p when and
    /// contains @p p. Returns `NotamSeverity::Info` if none match.
    NotamSeverity worstAt(const geo::GeoPoint& p,
                          std::chrono::system_clock::time_point when) const;

    /// True when @p p is inside any active NOTAM at time @p when.
    bool isAffected(const geo::GeoPoint& p,
                    std::chrono::system_clock::time_point when) const;

private:
    std::vector<Notam> _items;
};

} // namespace m130::weather

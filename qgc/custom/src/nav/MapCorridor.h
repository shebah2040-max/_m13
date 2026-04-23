#pragma once

#include "../geo/Wgs84.h"

#include <optional>
#include <vector>

namespace m130::nav {

/// Range-safety map corridor. Built from either:
///   1. A geodetic polygon (ray-casting test).
///   2. A centreline track + half-width (perpendicular cross-track
///      distance via `geo::crossTrackDistance`).
///
/// Both representations may coexist — a point is "inside" when BOTH the
/// polygon (if any) contains it AND no centreline leg rejects it.
class MapCorridor
{
public:
    struct TrackLeg {
        geo::GeoPoint a;
        geo::GeoPoint b;
        double half_width_m = 0.0;
    };

    /// Replace the polygon. Pass an empty vector to clear.
    void setPolygon(std::vector<geo::GeoPoint> vertices);

    /// Add a centreline leg. Half-width > 0 required.
    void addTrackLeg(const geo::GeoPoint& a, const geo::GeoPoint& b, double half_width_m);

    /// Clear both polygon and legs.
    void clear();

    bool hasPolygon() const noexcept { return !_polygon.empty(); }
    bool hasTrack()   const noexcept { return !_legs.empty(); }

    /// Test membership.
    bool contains(const geo::GeoPoint& p) const;

    /// Signed margin in metres. Positive values mean "inside" (distance
    /// to the nearest boundary); negative values mean "outside" (magnitude
    /// = shortest distance to any boundary). `std::nullopt` when the
    /// corridor is empty.
    std::optional<double> marginMeters(const geo::GeoPoint& p) const;

    const std::vector<geo::GeoPoint>& polygon() const noexcept { return _polygon; }
    const std::vector<TrackLeg>&      legs()    const noexcept { return _legs; }

private:
    bool _polygonContains(const geo::GeoPoint& p) const;

    std::vector<geo::GeoPoint> _polygon;
    std::vector<TrackLeg>      _legs;
};

} // namespace m130::nav

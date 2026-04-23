#pragma once

#include "../geo/Wgs84.h"

#include <chrono>
#include <cstddef>
#include <deque>
#include <optional>
#include <vector>

namespace m130::nav {

/// Sampled 3D state from the vehicle. Position is geodetic; velocity is
/// the local ENU (east, north, up) rate at that instant.
struct TrajectorySample {
    std::chrono::system_clock::time_point t{};
    geo::GeoPoint   position;
    double          ve_mps = 0.0; ///< east  rate
    double          vn_mps = 0.0; ///< north rate
    double          vu_mps = 0.0; ///< up    rate
};

/// Axis-aligned bounding box in ENU metres relative to a fixed origin.
struct EnuBounds {
    bool   valid = false;
    double min_east_m  = 0.0;
    double max_east_m  = 0.0;
    double min_north_m = 0.0;
    double max_north_m = 0.0;
    double min_up_m    = 0.0;
    double max_up_m    = 0.0;
};

/// Ring buffer of 3D trajectory samples + IIP history, with ENU bounds
/// computed around a fixed origin (the launch pad).
///
/// Pure C++, stdlib-only; the Qt wrapper (`TrajectoryController`) binds
/// this to QML.
class TrajectoryModel
{
public:
    TrajectoryModel();

    /// Set the origin used for ENU computations. Reset clears history.
    void setOrigin(const geo::GeoPoint& origin);

    /// Max retained samples. When exceeded, oldest are dropped.
    void setCapacity(std::size_t n);
    std::size_t capacity() const noexcept { return _cap; }

    /// Append a live sample. Rejected when monotonically older than the
    /// previous sample (replay engines run forwards only here).
    bool append(const TrajectorySample& s);

    /// Append a predicted IIP (impact point). IIPs are stored separately
    /// from the flown track but share the same bounds.
    void appendIip(std::chrono::system_clock::time_point t,
                   const geo::GeoPoint& iip);

    void clear();

    const std::deque<TrajectorySample>& samples() const noexcept { return _samples; }
    const std::deque<TrajectorySample>& iips()    const noexcept { return _iips; }
    const geo::GeoPoint&                origin()  const noexcept { return _origin; }
    const EnuBounds&                    bounds()  const noexcept { return _bounds; }
    bool                                hasOrigin() const noexcept { return _has_origin; }

    /// ENU offset of the most recent sample (if any).
    std::optional<geo::EnuOffset> lastEnu() const;

    /// Downsample the track to at most @p max_points samples using
    /// even-stride selection. Useful to feed a QML Canvas.
    std::vector<geo::EnuOffset> trackEnu(std::size_t max_points = 1024) const;

private:
    void _refreshBounds();

    std::size_t                  _cap = 4096;
    geo::GeoPoint                _origin;
    bool                         _has_origin = false;
    std::deque<TrajectorySample> _samples;
    std::deque<TrajectorySample> _iips;
    EnuBounds                    _bounds;
};

} // namespace m130::nav

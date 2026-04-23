#include "TrajectoryModel.h"

#include <algorithm>

namespace m130::nav {

TrajectoryModel::TrajectoryModel() = default;

void TrajectoryModel::setOrigin(const geo::GeoPoint& origin)
{
    _origin     = origin;
    _has_origin = true;
    clear();
}

void TrajectoryModel::setCapacity(std::size_t n)
{
    _cap = n == 0 ? 1 : n;
    while (_samples.size() > _cap) _samples.pop_front();
    while (_iips.size()    > _cap) _iips.pop_front();
    _refreshBounds();
}

bool TrajectoryModel::append(const TrajectorySample& s)
{
    if (!_samples.empty() && s.t < _samples.back().t) {
        return false;
    }
    _samples.push_back(s);
    while (_samples.size() > _cap) _samples.pop_front();
    _refreshBounds();
    return true;
}

void TrajectoryModel::appendIip(std::chrono::system_clock::time_point t,
                                const geo::GeoPoint& iip)
{
    TrajectorySample s;
    s.t = t;
    s.position = iip;
    _iips.push_back(s);
    while (_iips.size() > _cap) _iips.pop_front();
    _refreshBounds();
}

void TrajectoryModel::clear()
{
    _samples.clear();
    _iips.clear();
    _bounds = {};
}

std::optional<geo::EnuOffset> TrajectoryModel::lastEnu() const
{
    if (!_has_origin || _samples.empty()) return std::nullopt;
    return geo::llaToEnu(_samples.back().position, _origin);
}

std::vector<geo::EnuOffset>
TrajectoryModel::trackEnu(std::size_t max_points) const
{
    std::vector<geo::EnuOffset> out;
    if (!_has_origin || _samples.empty() || max_points == 0) return out;

    const std::size_t n = _samples.size();
    const std::size_t stride = n <= max_points
        ? 1
        : (n + max_points - 1) / max_points;

    out.reserve((n + stride - 1) / stride);
    for (std::size_t i = 0; i < n; i += stride) {
        out.push_back(geo::llaToEnu(_samples[i].position, _origin));
    }
    if (((n - 1) % stride) != 0) {
        out.push_back(geo::llaToEnu(_samples.back().position, _origin));
    }
    return out;
}

void TrajectoryModel::_refreshBounds()
{
    _bounds = {};
    if (!_has_origin) return;

    auto extend = [&](const geo::GeoPoint& p) {
        const auto e = geo::llaToEnu(p, _origin);
        if (!_bounds.valid) {
            _bounds.valid = true;
            _bounds.min_east_m  = _bounds.max_east_m  = e.east_m;
            _bounds.min_north_m = _bounds.max_north_m = e.north_m;
            _bounds.min_up_m    = _bounds.max_up_m    = e.up_m;
            return;
        }
        _bounds.min_east_m  = std::min(_bounds.min_east_m,  e.east_m);
        _bounds.max_east_m  = std::max(_bounds.max_east_m,  e.east_m);
        _bounds.min_north_m = std::min(_bounds.min_north_m, e.north_m);
        _bounds.max_north_m = std::max(_bounds.max_north_m, e.north_m);
        _bounds.min_up_m    = std::min(_bounds.min_up_m,    e.up_m);
        _bounds.max_up_m    = std::max(_bounds.max_up_m,    e.up_m);
    };
    for (const auto& s : _samples) extend(s.position);
    for (const auto& s : _iips)    extend(s.position);
}

} // namespace m130::nav

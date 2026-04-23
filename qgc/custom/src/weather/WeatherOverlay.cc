#include "WeatherOverlay.h"

#include <algorithm>
#include <cmath>

namespace m130::weather {

namespace {

constexpr double kPi  = 3.141592653589793238462643383279502884;
constexpr double kD2r = kPi / 180.0;

/// Convert a met-wind (FROM dir, speed) to ENU (east, north) components.
/// Meteorology: wind coming FROM θ, moving TO θ+180°.
void metToEn(double dir_deg, double speed_mps, double& u_east, double& v_north) noexcept
{
    const double to_rad = (dir_deg + 180.0) * kD2r;
    u_east  = speed_mps * std::sin(to_rad);
    v_north = speed_mps * std::cos(to_rad);
}

} // namespace

void WeatherOverlay::setBounds(double min_lat_deg, double min_lon_deg,
                               double max_lat_deg, double max_lon_deg) noexcept
{
    _min_lat = min_lat_deg;
    _min_lon = min_lon_deg;
    _max_lat = max_lat_deg;
    _max_lon = max_lon_deg;
}

bool WeatherOverlay::setCells(std::size_t rows, std::size_t cols,
                              std::vector<WeatherCell> cells)
{
    if (rows < 2 || cols < 2) return false;
    if (cells.size() != rows * cols) return false;
    _rows  = rows;
    _cols  = cols;
    _cells = std::move(cells);
    return true;
}

WeatherSample WeatherOverlay::sampleAt(double lat_deg, double lon_deg) const
{
    WeatherSample out;
    if (_cells.empty() || _rows < 2 || _cols < 2) return out;
    if (_max_lat <= _min_lat || _max_lon <= _min_lon) return out;

    const double fx = std::clamp((lon_deg - _min_lon) / (_max_lon - _min_lon), 0.0, 1.0);
    const double fy = std::clamp((lat_deg - _min_lat) / (_max_lat - _min_lat), 0.0, 1.0);

    const double gx = fx * static_cast<double>(_cols - 1);
    const double gy = fy * static_cast<double>(_rows - 1);
    const std::size_t i0 = static_cast<std::size_t>(std::floor(gx));
    const std::size_t j0 = static_cast<std::size_t>(std::floor(gy));
    const std::size_t i1 = std::min(i0 + 1, _cols - 1);
    const std::size_t j1 = std::min(j0 + 1, _rows - 1);
    const double tx = gx - static_cast<double>(i0);
    const double ty = gy - static_cast<double>(j0);

    auto at = [&](std::size_t r, std::size_t c) -> const WeatherCell& {
        return _cells[r * _cols + c];
    };
    const WeatherCell& a = at(j0, i0);
    const WeatherCell& b = at(j0, i1);
    const WeatherCell& c = at(j1, i0);
    const WeatherCell& d = at(j1, i1);

    double ua, va, ub, vb, uc, vc, ud, vd;
    metToEn(a.wind_dir_deg, a.wind_speed_mps, ua, va);
    metToEn(b.wind_dir_deg, b.wind_speed_mps, ub, vb);
    metToEn(c.wind_dir_deg, c.wind_speed_mps, uc, vc);
    metToEn(d.wind_dir_deg, d.wind_speed_mps, ud, vd);

    auto lerp = [](double p00, double p10, double p01, double p11, double sx, double sy) {
        return (1.0 - sx) * (1.0 - sy) * p00
             +        sx  * (1.0 - sy) * p10
             + (1.0 - sx) *        sy  * p01
             +        sx  *        sy  * p11;
    };

    out.u_east_mps   = lerp(ua, ub, uc, ud, tx, ty);
    out.v_north_mps  = lerp(va, vb, vc, vd, tx, ty);
    out.pressure_pa  = lerp(a.pressure_pa,  b.pressure_pa,
                            c.pressure_pa,  d.pressure_pa,  tx, ty);
    out.temperature_k = lerp(a.temperature_k, b.temperature_k,
                             c.temperature_k, d.temperature_k, tx, ty);
    out.valid = true;
    return out;
}

} // namespace m130::weather

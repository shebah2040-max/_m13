#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace m130::weather {

/// Structured wind / pressure / temperature sample at a geographic
/// grid cell.
struct WeatherCell {
    double wind_dir_deg  = 0.0; ///< meteorological FROM direction (0 = N)
    double wind_speed_mps = 0.0;
    double pressure_pa   = 0.0;
    double temperature_k = 0.0;
};

struct WeatherSample {
    double u_east_mps  = 0.0; ///< east wind component
    double v_north_mps = 0.0; ///< north wind component
    double pressure_pa = 0.0;
    double temperature_k = 0.0;
    bool   valid       = false;
};

/// Regular lat/lon grid of weather cells. Bilinearly interpolated on
/// query. Edges clamp rather than extrapolate. The grid is lat-major,
/// row 0 = min_lat, column 0 = min_lon.
class WeatherOverlay
{
public:
    void setBounds(double min_lat_deg, double min_lon_deg,
                   double max_lat_deg, double max_lon_deg) noexcept;

    /// Resize and replace the grid. `cells.size()` must equal rows*cols.
    bool setCells(std::size_t rows, std::size_t cols, std::vector<WeatherCell> cells);

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }
    bool        empty() const noexcept { return _cells.empty(); }

    /// Interpolate at geographic location. Returns `valid=false` if
    /// no grid has been set.
    WeatherSample sampleAt(double lat_deg, double lon_deg) const;

private:
    double _min_lat = 0.0, _max_lat = 0.0;
    double _min_lon = 0.0, _max_lon = 0.0;
    std::size_t _rows = 0, _cols = 0;
    std::vector<WeatherCell> _cells;
};

} // namespace m130::weather

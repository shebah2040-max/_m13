#include "IipPredictor.h"

#include <cmath>

namespace m130::views {

IipResult IipPredictor::predict(double pos_down_m, double pos_cross_m, double pos_up_m,
                                double vel_down_m_s, double vel_cross_m_s, double vel_up_m_s,
                                double g) noexcept
{
    IipResult out;
    if (!(g > 0.0)) return out;

    // Vertical ballistics: z(t) = pos_up + vz * t - 0.5 * g * t^2.
    // Solve z(t) = 0 for t > 0 → t = (vz + sqrt(vz^2 + 2 g pos_up)) / g.
    const double disc = vel_up_m_s * vel_up_m_s + 2.0 * g * pos_up_m;
    if (disc < 0.0) return out;

    const double t = (vel_up_m_s + std::sqrt(disc)) / g;
    if (!(t > 0.0) || !std::isfinite(t)) return out;

    out.impact_downrange_m  = pos_down_m  + vel_down_m_s  * t;
    out.impact_crossrange_m = pos_cross_m + vel_cross_m_s * t;
    out.time_to_impact_s    = t;
    out.valid = true;
    return out;
}

} // namespace m130::views

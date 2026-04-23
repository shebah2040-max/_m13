#include "Wgs84.h"

#include <cmath>

namespace m130::geo {

namespace {

constexpr double kPi   = 3.141592653589793238462643383279502884;
constexpr double kD2r  = kPi / 180.0;
constexpr double kR2d  = 180.0 / kPi;

inline double sqr(double x) noexcept { return x * x; }

} // namespace

EcefPoint llaToEcef(const GeoPoint& p)
{
    const double lat = p.lat_deg * kD2r;
    const double lon = p.lon_deg * kD2r;
    const double sl = std::sin(lat);
    const double cl = std::cos(lat);
    const double so = std::sin(lon);
    const double co = std::cos(lon);
    const double N = wgs84::kA / std::sqrt(1.0 - wgs84::kE2 * sl * sl);

    EcefPoint out;
    out.x = (N + p.alt_m) * cl * co;
    out.y = (N + p.alt_m) * cl * so;
    out.z = (N * (1.0 - wgs84::kE2) + p.alt_m) * sl;
    return out;
}

GeoPoint ecefToLla(const EcefPoint& e)
{
    // Bowring 1985 closed-form.
    const double p = std::sqrt(e.x * e.x + e.y * e.y);
    const double theta = std::atan2(e.z * wgs84::kA, p * wgs84::kB);
    const double st = std::sin(theta);
    const double ct = std::cos(theta);

    const double lat = std::atan2(e.z + wgs84::kEP2 * wgs84::kB * st * st * st,
                                  p  - wgs84::kE2  * wgs84::kA * ct * ct * ct);
    const double lon = std::atan2(e.y, e.x);
    const double sl = std::sin(lat);
    const double N = wgs84::kA / std::sqrt(1.0 - wgs84::kE2 * sl * sl);

    GeoPoint out;
    out.lat_deg = lat * kR2d;
    out.lon_deg = lon * kR2d;
    out.alt_m   = p / std::cos(lat) - N;
    return out;
}

EnuOffset llaToEnu(const GeoPoint& p, const GeoPoint& origin)
{
    const EcefPoint e_p   = llaToEcef(p);
    const EcefPoint e_o   = llaToEcef(origin);
    const double dx = e_p.x - e_o.x;
    const double dy = e_p.y - e_o.y;
    const double dz = e_p.z - e_o.z;

    const double lat = origin.lat_deg * kD2r;
    const double lon = origin.lon_deg * kD2r;
    const double sl = std::sin(lat);
    const double cl = std::cos(lat);
    const double so = std::sin(lon);
    const double co = std::cos(lon);

    EnuOffset out;
    out.east_m  = -so * dx + co * dy;
    out.north_m = -sl * co * dx - sl * so * dy + cl * dz;
    out.up_m    =  cl * co * dx + cl * so * dy + sl * dz;
    return out;
}

GeoPoint enuToLla(const EnuOffset& o, const GeoPoint& origin)
{
    const EcefPoint e_o = llaToEcef(origin);
    const double lat = origin.lat_deg * kD2r;
    const double lon = origin.lon_deg * kD2r;
    const double sl = std::sin(lat);
    const double cl = std::cos(lat);
    const double so = std::sin(lon);
    const double co = std::cos(lon);

    // Transpose of the ENU rotation applied in llaToEnu.
    const double dx = -so * o.east_m - sl * co * o.north_m + cl * co * o.up_m;
    const double dy =  co * o.east_m - sl * so * o.north_m + cl * so * o.up_m;
    const double dz =                   cl      * o.north_m + sl      * o.up_m;

    EcefPoint e;
    e.x = e_o.x + dx;
    e.y = e_o.y + dy;
    e.z = e_o.z + dz;
    return ecefToLla(e);
}

// Suppress unused-function warning for sqr() on some compilers.
[[maybe_unused]] constexpr auto _keep_sqr = &sqr;

} // namespace m130::geo

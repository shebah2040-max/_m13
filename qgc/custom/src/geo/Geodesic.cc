#include "Geodesic.h"

#include <cmath>

namespace m130::geo {

namespace {

constexpr double kPi   = 3.141592653589793238462643383279502884;
constexpr double kD2r  = kPi / 180.0;
constexpr double kR2d  = 180.0 / kPi;
constexpr double kR    = 6371008.7714; ///< WGS84 mean radius [m] (IUGG)

inline double norm2pi(double a) noexcept
{
    while (a < 0.0)        a += 2.0 * kPi;
    while (a >= 2.0 * kPi) a -= 2.0 * kPi;
    return a;
}

} // namespace

double haversineDistance(const GeoPoint& a, const GeoPoint& b)
{
    const double la1 = a.lat_deg * kD2r;
    const double la2 = b.lat_deg * kD2r;
    const double dlat = (b.lat_deg - a.lat_deg) * kD2r;
    const double dlon = (b.lon_deg - a.lon_deg) * kD2r;
    const double h = std::sin(dlat * 0.5) * std::sin(dlat * 0.5)
                   + std::cos(la1) * std::cos(la2)
                     * std::sin(dlon * 0.5) * std::sin(dlon * 0.5);
    const double c = 2.0 * std::asin(std::min(1.0, std::sqrt(h)));
    return kR * c;
}

double initialBearing(const GeoPoint& a, const GeoPoint& b)
{
    const double la1 = a.lat_deg * kD2r;
    const double la2 = b.lat_deg * kD2r;
    const double dlon = (b.lon_deg - a.lon_deg) * kD2r;
    const double y = std::sin(dlon) * std::cos(la2);
    const double x = std::cos(la1) * std::sin(la2)
                   - std::sin(la1) * std::cos(la2) * std::cos(dlon);
    return norm2pi(std::atan2(y, x));
}

GeoPoint destination(const GeoPoint& origin, double bearing_rad, double distance_m)
{
    const double la1 = origin.lat_deg * kD2r;
    const double lo1 = origin.lon_deg * kD2r;
    const double ang = distance_m / kR;

    const double sla = std::sin(la1);
    const double cla = std::cos(la1);
    const double sa  = std::sin(ang);
    const double ca  = std::cos(ang);

    const double la2 = std::asin(sla * ca + cla * sa * std::cos(bearing_rad));
    const double lo2 = lo1 + std::atan2(std::sin(bearing_rad) * sa * cla,
                                        ca - sla * std::sin(la2));

    GeoPoint out;
    out.lat_deg = la2 * kR2d;
    out.lon_deg = lo2 * kR2d;
    out.alt_m   = origin.alt_m;
    return out;
}

double crossTrackDistance(const GeoPoint& a, const GeoPoint& b, const GeoPoint& p)
{
    const double d13 = haversineDistance(a, p) / kR;
    const double t13 = initialBearing(a, p);
    const double t12 = initialBearing(a, b);
    return std::asin(std::sin(d13) * std::sin(t13 - t12)) * kR;
}

} // namespace m130::geo

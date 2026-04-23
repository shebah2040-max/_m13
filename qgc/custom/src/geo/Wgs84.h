#pragma once

#include <array>

namespace m130::geo {

/// WGS84 ellipsoid constants (EPSG:4326).
namespace wgs84 {
constexpr double kA = 6378137.0;               ///< semi-major axis [m]
constexpr double kF = 1.0 / 298.257223563;     ///< flattening
constexpr double kB = kA * (1.0 - kF);         ///< semi-minor axis
constexpr double kE2 = kF * (2.0 - kF);        ///< first eccentricity squared
constexpr double kEP2 = kE2 / (1.0 - kE2);     ///< second eccentricity squared
} // namespace wgs84

/// Geodetic coordinate: latitude/longitude in **degrees**, altitude in metres
/// above the WGS84 ellipsoid.
struct GeoPoint {
    double lat_deg = 0.0;
    double lon_deg = 0.0;
    double alt_m   = 0.0;
};

/// Earth-centered, earth-fixed Cartesian coordinate (metres).
struct EcefPoint {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

/// Local tangent plane (east/north/up) offset in metres.
struct EnuOffset {
    double east_m  = 0.0;
    double north_m = 0.0;
    double up_m    = 0.0;
};

/// LLA -> ECEF.
EcefPoint llaToEcef(const GeoPoint& p);

/// ECEF -> LLA via Bowring's closed-form. Accurate to < 1 mm at
/// altitudes between -10 km and +100 km.
GeoPoint  ecefToLla(const EcefPoint& e);

/// ENU offset of @p p expressed relative to @p origin.
EnuOffset llaToEnu(const GeoPoint& p, const GeoPoint& origin);

/// Reconstruct an LLA point from an ENU offset relative to @p origin.
GeoPoint  enuToLla(const EnuOffset& o, const GeoPoint& origin);

} // namespace m130::geo

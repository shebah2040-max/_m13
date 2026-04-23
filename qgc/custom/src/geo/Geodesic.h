#pragma once

#include "Wgs84.h"

namespace m130::geo {

/// Great-circle distance on the WGS84 sphere (mean radius).
/// Accurate to ~0.3% for distances up to a few thousand kilometres.
double haversineDistance(const GeoPoint& a, const GeoPoint& b);

/// Initial bearing [rad, 0..2π] from @p a to @p b along a great circle.
double initialBearing(const GeoPoint& a, const GeoPoint& b);

/// Destination point from @p origin after travelling @p distance_m on
/// initial bearing @p bearing_rad (spherical Earth).
GeoPoint destination(const GeoPoint& origin, double bearing_rad, double distance_m);

/// Perpendicular cross-track distance [m] from @p p to the great-circle
/// segment a→b. Positive = right of track, negative = left.
double crossTrackDistance(const GeoPoint& a, const GeoPoint& b, const GeoPoint& p);

} // namespace m130::geo

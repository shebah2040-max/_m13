# Pillar 8 — 3D Trajectory + Map Corridor + Weather/NOTAM

Pillar 8 turns the telemetry stream into a geospatial picture that
range safety can read at a glance: a live 3D track, a corridor state
machine with hysteresis, a bilinearly-interpolated weather overlay, and
a NOTAM index.

## Scope

| Layer              | File(s)                                       | CI suite       |
|--------------------|-----------------------------------------------|----------------|
| Geodetic math      | `geo/Wgs84.{h,cc}`, `geo/Geodesic.{h,cc}`     | m130_core      |
| Trajectory history | `nav/TrajectoryModel.{h,cc}`                  | m130_core      |
| Corridor policy    | `nav/MapCorridor.{h,cc}`                      | m130_core      |
| Corridor monitor   | `nav/CorridorMonitor.{h,cc}`                  | m130_core      |
| Weather grid       | `weather/WeatherOverlay.{h,cc}`               | m130_core      |
| NOTAM model        | `weather/NotamModel.{h,cc}`                   | m130_core      |
| Qt-bound glue      | `nav/TrajectoryController.{h,cc}`, `nav/MapController.{h,cc}`, `weather/WeatherController.{h,cc}` | main build |
| QML                | `views/trajectory/Trajectory3DView.qml`, `views/map/MapCorridorOverlay.qml`, `views/weather/WeatherBadge.qml` | main build |

## 1. Geodetic math (`geo/`)

### `Wgs84`

- `llaToEcef(lat,lon,alt) -> (x,y,z)` using the standard EPSG:4326
  forward formula.
- `ecefToLla(x,y,z) -> (lat,lon,alt)` via Bowring's 1985 closed-form.
  Round-trip error is below 1 mm at altitudes between −10 km and
  +100 km.
- `llaToEnu(p, origin)` / `enuToLla(offset, origin)` expose the local
  tangent-plane frame every downstream core uses.

### `Geodesic`

Great-circle operations for flat-earth-friendly UI logic:

- `haversineDistance(a, b)` — mean-radius great-circle distance.
- `initialBearing(a, b)` — returns radians in `[0, 2π)`.
- `destination(origin, bearing, dist)` — the inverse of the above pair.
- `crossTrackDistance(a, b, p)` — signed perpendicular distance of
  point `p` from the great-circle segment `a→b`.

### Accuracy notes

The WGS84 formulas are strict; the haversine helpers use a single
mean-radius sphere, which is fine for all corridor sizes we care about
(tens to hundreds of kilometres) but would not be suitable for long-haul
route planning. Corridor margin reporting assumes the great-circle
approximation is accurate enough near the boundary.

## 2. Trajectory history (`nav/TrajectoryModel`)

- Ring buffer of `TrajectorySample` (position + ENU velocity +
  timestamp) with a configurable capacity (default 4096).
- Rejects monotonically-older samples.
- Tracks predicted IIPs in a separate ring buffer; both feed the
  shared ENU bounding box.
- `trackEnu(max_points)` downsamples with a stride so a QML Canvas can
  draw a fixed-budget polyline.

## 3. Corridor (`nav/MapCorridor`, `nav/CorridorMonitor`)

### `MapCorridor`

A corridor may be defined two ways, combinable:

1. **Polygon** — any number of geodetic vertices. Membership via
   ray-casting in lat/lon space.
2. **Centreline legs** — each leg is `(a, b, half_width_m)`. A point is
   inside a leg when `|crossTrackDistance| <= half_width_m`.

A point is inside the corridor when both constraints pass (the polygon
contains it AND no leg rejects it). `marginMeters` returns a signed
distance in metres to the nearest boundary — positive inside, negative
outside — which the UI uses to drive warning bands.

### `CorridorMonitor`

State machine with hysteresis and persistence timers. Four states:

- `Unknown` — no corridor installed, or a live sample hasn't arrived.
- `Inside` — comfortably inside.
- `Warning` — inside but margin below `warn_margin_m`.
- `Breach` — outside the corridor for at least `breach_persistence`.

Returning to `Inside` requires the margin to exceed `clear_margin_m`
(defaults larger than `warn_margin_m`). `just_breached` and
`just_cleared` flags fire once per transition so range safety can raise
and retract AlertManager alerts without dedicated edge detection.

## 4. Weather + NOTAM (`weather/`)

### `WeatherOverlay`

Regular lat/lon grid of cells (wind direction/speed, pressure,
temperature). `sampleAt(lat, lon)` converts the cell's wind dir/speed to
ENU components (east/north), bilinearly interpolates the four
neighbouring cells, and returns the ENU wind vector plus interpolated
pressure/temperature. Edges clamp rather than extrapolate.

### `NotamModel`

Append-only list of NOTAMs with structured validity:

- Time validity via `[start, end]` range.
- Area via either polygon or (centre + radius) circle. `NotamArea`
  hides the discriminator.
- Severity as a 4-level enum (Info, Advisory, Warning, Hazard).

Queries: `activeAt(time)`, `isAffected(point, time)`, and
`worstAt(point, time)` — the last is used by `WeatherBadge.qml` to pick
the badge colour.

## 5. Qt-bound controllers

Each core has a Qt facade exposed to QML:

- `TrajectoryController` — drains `M130_GNC_STATE` samples from
  `CustomPlugin::mavlinkMessage()` into `TrajectoryModel` and
  publishes a downsampled ENU track via `TrajectoryTrackModel`
  (`QAbstractListModel`).
- `MapController` — edits the corridor polygon/legs from QML, runs
  `CorridorMonitor` on each live fix, and signals
  `corridorBreached/Cleared`.
- `WeatherController` — wraps both weather + NOTAM models. JSON
  feeders push grids and NOTAM lists in; `sampleWeather(lat,lon)` and
  `worstNotamSeverity(lat,lon,t)` fan out to the UI.

## 6. QML

- `Trajectory3DView.qml` — `Canvas` top-down view of the track with
  auto-fit bounds. Cesium/Qt Quick 3D is planned for Phase 8b.
- `MapCorridorOverlay.qml` — compact status strip with margin read-out
  and colour-coded border tied to `CorridorState`.
- `WeatherBadge.qml` — live wind read-out + worst NOTAM severity at the
  launch pad.

## 7. CI impact

| Suite                         | Before 8 | After 8 |
|-------------------------------|----------|---------|
| `m130_core` pure-C++ tests    | 44       | 51      |

New tests: `Wgs84Test` (6), `GeodesicTest` (6), `TrajectoryModelTest`
(6), `MapCorridorTest` (7), `CorridorMonitorTest` (4),
`WeatherOverlayTest` (5), `NotamModelTest` (5).

## 8. Deferred to Phase 8b

- Cesium / Qt Quick 3D 3D scene with terrain tiles.
- Offline tile cache + configurable basemap.
- Wind-corrected IIP predictor (feed `WeatherOverlay` into
  `IipPredictor`).
- Live NOTAM / METAR / TAF feed clients (HTTP + JSON).
- Map corridor editor UI (draw/import polygons from GeoJSON).

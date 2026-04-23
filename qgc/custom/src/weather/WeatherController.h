#pragma once

#include "NotamModel.h"
#include "WeatherOverlay.h"

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

namespace m130::gui {

/// Qt-bound facade for `weather::WeatherOverlay` + `weather::NotamModel`.
///
/// Consumed by `MapCorridorOverlay.qml` and the RangeSafety / PreLaunch
/// consoles. Data is pushed in from JSON feeds elsewhere in the app; the
/// controller does not fetch from the network itself.
class WeatherController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool weatherAvailable READ weatherAvailable NOTIFY weatherChanged)
    Q_PROPERTY(int  notamCount       READ notamCount       NOTIFY notamChanged)

public:
    explicit WeatherController(QObject* parent = nullptr);
    ~WeatherController() override;

    weather::WeatherOverlay& weather() noexcept { return _weather; }
    weather::NotamModel&     notams()  noexcept { return _notams; }

    bool weatherAvailable() const noexcept { return !_weather.empty(); }
    int  notamCount()       const noexcept { return static_cast<int>(_notams.size()); }

public slots:
    /// Replace the weather grid. Cells is a flat list of rows*cols JSON
    /// objects each with keys: wind_dir_deg, wind_speed_mps,
    /// pressure_pa, temperature_k.
    bool setWeatherGrid(double min_lat, double min_lon,
                        double max_lat, double max_lon,
                        int rows, int cols,
                        const QVariantList& cells);

    /// Sample the weather overlay at a geographic point. Returns an
    /// empty map when no grid is set.
    QVariantMap sampleWeather(double lat, double lon) const;

    /// Replace all NOTAMs.
    void setNotams(const QVariantList& items);
    void clearNotams();

    /// Return the worst severity (0..3) active at @p time_ms containing
    /// @p lat,@p lon.
    int worstNotamSeverity(double lat, double lon, qint64 time_ms) const;

signals:
    void weatherChanged();
    void notamChanged();

private:
    weather::WeatherOverlay _weather;
    weather::NotamModel     _notams;
};

} // namespace m130::gui

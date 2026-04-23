#include "WeatherController.h"

#include <QtCore/QPointF>
#include <QtCore/QString>

#include <chrono>

namespace m130::gui {

namespace {

std::chrono::system_clock::time_point msToTp(qint64 ms)
{
    return std::chrono::system_clock::time_point{std::chrono::milliseconds(ms)};
}

weather::NotamSeverity severityFromInt(int v)
{
    switch (v) {
    case 1: return weather::NotamSeverity::Advisory;
    case 2: return weather::NotamSeverity::Warning;
    case 3: return weather::NotamSeverity::Hazard;
    default: return weather::NotamSeverity::Info;
    }
}

} // namespace

WeatherController::WeatherController(QObject* parent) : QObject(parent) {}
WeatherController::~WeatherController() = default;

bool WeatherController::setWeatherGrid(double min_lat, double min_lon,
                                       double max_lat, double max_lon,
                                       int rows, int cols,
                                       const QVariantList& cells)
{
    if (rows < 2 || cols < 2) return false;
    if (cells.size() != rows * cols) return false;

    _weather.setBounds(min_lat, min_lon, max_lat, max_lon);
    std::vector<weather::WeatherCell> out;
    out.reserve(cells.size());
    for (const auto& v : cells) {
        const QVariantMap m = v.toMap();
        weather::WeatherCell c;
        c.wind_dir_deg   = m.value(QStringLiteral("wind_dir_deg"),   0.0).toDouble();
        c.wind_speed_mps = m.value(QStringLiteral("wind_speed_mps"), 0.0).toDouble();
        c.pressure_pa    = m.value(QStringLiteral("pressure_pa"),    101325.0).toDouble();
        c.temperature_k  = m.value(QStringLiteral("temperature_k"),  288.15).toDouble();
        out.push_back(c);
    }
    const bool ok = _weather.setCells(static_cast<std::size_t>(rows),
                                      static_cast<std::size_t>(cols),
                                      std::move(out));
    if (ok) emit weatherChanged();
    return ok;
}

QVariantMap WeatherController::sampleWeather(double lat, double lon) const
{
    const auto s = _weather.sampleAt(lat, lon);
    QVariantMap out;
    out[QStringLiteral("valid")] = s.valid;
    if (!s.valid) return out;
    out[QStringLiteral("u_east_mps")]    = s.u_east_mps;
    out[QStringLiteral("v_north_mps")]   = s.v_north_mps;
    out[QStringLiteral("pressure_pa")]   = s.pressure_pa;
    out[QStringLiteral("temperature_k")] = s.temperature_k;
    return out;
}

void WeatherController::setNotams(const QVariantList& items)
{
    _notams.clear();
    for (const auto& v : items) {
        const QVariantMap m = v.toMap();
        weather::Notam n;
        n.id       = m.value(QStringLiteral("id")).toString().toStdString();
        n.summary  = m.value(QStringLiteral("summary")).toString().toStdString();
        n.severity = severityFromInt(m.value(QStringLiteral("severity"), 0).toInt());
        n.start = msToTp(m.value(QStringLiteral("start_ms"), 0).toLongLong());
        n.end   = msToTp(m.value(QStringLiteral("end_ms"),   0).toLongLong());

        if (m.contains(QStringLiteral("polygon"))) {
            const QVariantList pts = m.value(QStringLiteral("polygon")).toList();
            for (const auto& p : pts) {
                const QPointF pt = p.toPointF();
                n.area.polygon.push_back({pt.x(), pt.y(), 0.0});
            }
        }
        n.area.centre   = {
            m.value(QStringLiteral("centre_lat"), 0.0).toDouble(),
            m.value(QStringLiteral("centre_lon"), 0.0).toDouble(),
            0.0
        };
        n.area.radius_m = m.value(QStringLiteral("radius_m"), 0.0).toDouble();
        _notams.add(std::move(n));
    }
    emit notamChanged();
}

void WeatherController::clearNotams()
{
    _notams.clear();
    emit notamChanged();
}

int WeatherController::worstNotamSeverity(double lat, double lon, qint64 time_ms) const
{
    return static_cast<int>(_notams.worstAt({lat, lon, 0.0}, msToTp(time_ms)));
}

} // namespace m130::gui

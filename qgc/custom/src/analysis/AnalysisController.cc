#include "AnalysisController.h"

#include <QtCore/QDateTime>

#include <utility>

namespace m130::gui {

AnalysisController::AnalysisController(QObject* parent)
    : QObject(parent)
{
}

AnalysisController::~AnalysisController() = default;

void AnalysisController::setSampleRateHz(double hz)
{
    if (hz <= 0.0 || hz == _sample_rate_hz) return;
    _sample_rate_hz = hz;
    emit sampleRateChanged();
}

void AnalysisController::setWindowTypeInt(int w)
{
    const auto nw = static_cast<analysis::WindowType>(w);
    if (nw == _window) return;
    _window = nw;
    emit windowChanged();
}

void AnalysisController::setActiveSeries(const QString& name)
{
    if (name == _active_series) return;
    _active_series = name;
    emit activeSeriesChanged();
}

void AnalysisController::installDefaultSeries()
{
    for (const char* n : {"phi", "theta", "psi", "q_dyn", "altitude",
                          "airspeed", "alpha_est", "mpc_solve_us"}) {
        _series(QString::fromLatin1(n), /*create*/ true);
    }
    if (_active_series.isEmpty()) {
        setActiveSeries(QStringLiteral("phi"));
    }
}

analysis::Timeseries* AnalysisController::_series(const QString& name, bool create)
{
    const auto key = name.toStdString();
    auto it = _streams.find(key);
    if (it == _streams.end()) {
        if (!create) return nullptr;
        it = _streams.emplace(key, analysis::Timeseries(key, kDefaultCapacity)).first;
    }
    return &it->second;
}

void AnalysisController::push(const QString& name, qint64 tMs, double value)
{
    auto* s = _series(name, true);
    if (!s) return;
    if (tMs < 0) tMs = 0;
    s->push(static_cast<std::uint64_t>(tMs), value);
    emit seriesUpdated(name);
}

void AnalysisController::pushInnovation(double normalised)
{
    _mhe.push(normalised);
    emit mheChanged();
}

QVariantList AnalysisController::downsample(int maxPoints) const
{
    QVariantList out;
    if (maxPoints <= 0 || _active_series.isEmpty()) return out;
    auto it = _streams.find(_active_series.toStdString());
    if (it == _streams.end()) return out;
    const auto samples = it->second.downsample(static_cast<std::size_t>(maxPoints));
    out.reserve(samples.size());
    for (const auto& s : samples) {
        QVariantMap m;
        m.insert(QStringLiteral("tMs"),  static_cast<qint64>(s.t_ms));
        m.insert(QStringLiteral("value"), s.value);
        out.append(m);
    }
    return out;
}

QVariantList AnalysisController::currentSpectrum() const
{
    QVariantList out;
    if (_active_series.isEmpty()) return out;
    auto it = _streams.find(_active_series.toStdString());
    if (it == _streams.end()) return out;
    const auto snap = std::vector<double>([&]{
        std::vector<double> v;
        v.reserve(it->second.size());
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            v.push_back(it->second.at(i).value);
        }
        return v;
    }());
    if (snap.size() < 8) return out;
    const auto s = analysis::computeSpectrum(snap, _sample_rate_hz, _window);
    out.reserve(static_cast<int>(s.frequencies_hz.size()));
    for (std::size_t k = 0; k < s.frequencies_hz.size(); ++k) {
        QVariantMap m;
        m.insert(QStringLiteral("fHz"), s.frequencies_hz[k]);
        m.insert(QStringLiteral("mag"), s.magnitude[k]);
        m.insert(QStringLiteral("psd"), s.psd[k]);
        out.append(m);
    }
    return out;
}

QVariantMap AnalysisController::currentSpectrumPeak() const
{
    QVariantMap out;
    if (_active_series.isEmpty()) return out;
    auto it = _streams.find(_active_series.toStdString());
    if (it == _streams.end()) return out;
    std::vector<double> v;
    v.reserve(it->second.size());
    for (std::size_t i = 0; i < it->second.size(); ++i) {
        v.push_back(it->second.at(i).value);
    }
    if (v.size() < 8) return out;
    const auto s = analysis::computeSpectrum(v, _sample_rate_hz, _window);
    const auto p = analysis::findPeak(s);
    out.insert(QStringLiteral("fHz"), p.frequency_hz);
    out.insert(QStringLiteral("mag"), p.magnitude);
    return out;
}

void AnalysisController::clearAll()
{
    for (auto& kv : _streams) kv.second.clear();
    _mhe.reset();
    emit mheChanged();
}

quint64 AnalysisController::mheSamples()     const { return _mhe.status().samples; }
double  AnalysisController::mheRunningMean() const { return _mhe.status().running_mean; }
double  AnalysisController::mheRunningVar()  const { return _mhe.status().running_var; }
double  AnalysisController::mheMaxAbsSeen()  const { return _mhe.status().max_abs_seen; }
bool    AnalysisController::mheMeanBias()    const { return _mhe.status().mean_bias; }
bool    AnalysisController::mheVarianceOut() const { return _mhe.status().variance_out; }
bool    AnalysisController::mheThreeSigma()  const { return _mhe.status().three_sigma; }
QString AnalysisController::mheReason()      const { return QString::fromStdString(_mhe.status().reason); }

} // namespace m130::gui

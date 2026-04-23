#pragma once

#include "MheInnovationMonitor.h"
#include "Spectrum.h"
#include "Timeseries.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>

#include <memory>
#include <string>
#include <unordered_map>

namespace m130::gui {

/// Qt-bound facade for the Pillar 7 analysis primitives.
///
/// Owns a set of named `analysis::Timeseries` buffers fed live by
/// `CustomPlugin::mavlinkMessage` (one per GNC scalar the operator cares
/// about) and a `MheInnovationMonitor`. Spectrum computation is on-demand:
/// callers ask for the current spectrum of a named series and receive a
/// `QVariantList` of `{f_hz, magnitude, psd}` triples suitable for a
/// `ListView` / `Canvas` plot.
class AnalysisController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double    sampleRateHz   READ sampleRateHz WRITE setSampleRateHz NOTIFY sampleRateChanged)
    Q_PROPERTY(int       windowType     READ windowTypeInt WRITE setWindowTypeInt NOTIFY windowChanged)
    Q_PROPERTY(QString   activeSeries   READ activeSeries WRITE setActiveSeries NOTIFY activeSeriesChanged)

    Q_PROPERTY(quint64   mheSamples     READ mheSamples    NOTIFY mheChanged)
    Q_PROPERTY(double    mheRunningMean READ mheRunningMean NOTIFY mheChanged)
    Q_PROPERTY(double    mheRunningVar  READ mheRunningVar  NOTIFY mheChanged)
    Q_PROPERTY(double    mheMaxAbsSeen  READ mheMaxAbsSeen  NOTIFY mheChanged)
    Q_PROPERTY(bool      mheMeanBias    READ mheMeanBias    NOTIFY mheChanged)
    Q_PROPERTY(bool      mheVarianceOut READ mheVarianceOut NOTIFY mheChanged)
    Q_PROPERTY(bool      mheThreeSigma  READ mheThreeSigma  NOTIFY mheChanged)
    Q_PROPERTY(QString   mheReason      READ mheReason      NOTIFY mheChanged)

public:
    explicit AnalysisController(QObject* parent = nullptr);
    ~AnalysisController() override;

    double  sampleRateHz() const noexcept { return _sample_rate_hz; }
    int     windowTypeInt() const noexcept { return static_cast<int>(_window); }
    QString activeSeries()  const noexcept { return _active_series; }

    quint64 mheSamples()     const;
    double  mheRunningMean() const;
    double  mheRunningVar()  const;
    double  mheMaxAbsSeen()  const;
    bool    mheMeanBias()    const;
    bool    mheVarianceOut() const;
    bool    mheThreeSigma()  const;
    QString mheReason()      const;

    void setSampleRateHz(double hz);
    void setWindowTypeInt(int w);
    void setActiveSeries(const QString& name);

    /// Install the default set of live-monitored series at 20 Hz capacity
    /// (512 samples / 25.6 s window). Safe to call once.
    Q_INVOKABLE void installDefaultSeries();

    /// Append one sample to a named timeseries; creates the series if it
    /// does not exist with the default capacity.
    Q_INVOKABLE void push(const QString& name, qint64 tMs, double value);

    /// Append one normalised innovation sample (ẑ = residual / sqrt(S)).
    Q_INVOKABLE void pushInnovation(double normalised);

    /// Returns a `QVariantList` of recent samples of the active series.
    /// Each entry: `{ t_ms, value }`. Up to @p maxPoints, uniformly spread.
    Q_INVOKABLE QVariantList downsample(int maxPoints = 256) const;

    /// Compute the current spectrum of the active series and return a list
    /// of `{f_hz, magnitude, psd}` triples. Empty when there is no data.
    Q_INVOKABLE QVariantList currentSpectrum() const;

    /// Peak of the current spectrum.
    Q_INVOKABLE QVariantMap  currentSpectrumPeak() const;

    /// Clear everything (all series + innovation monitor).
    Q_INVOKABLE void clearAll();

signals:
    void sampleRateChanged();
    void windowChanged();
    void activeSeriesChanged();
    void seriesUpdated(const QString& name);
    void mheChanged();

private:
    analysis::Timeseries* _series(const QString& name, bool create = true);

    mutable std::unordered_map<std::string, analysis::Timeseries> _streams;
    analysis::MheInnovationMonitor _mhe;
    double                   _sample_rate_hz = 20.0;
    analysis::WindowType     _window = analysis::WindowType::Hann;
    QString                  _active_series;
    static constexpr std::size_t kDefaultCapacity = 512;
};

} // namespace m130::gui

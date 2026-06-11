/**
 * @file Expander15.cpp
 * @brief Expander15 实现
 *
 * 实现动态扩展器：多频段包络跟随与心理声学加权动态范围增强实现响度最大化。
 */

#include "utils/dsp291/Expander15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Expander15::Expander15(QObject *parent)
    : QObject(parent)
{
    setNumBands(4);
    updateFilterCoeffs();
}

Expander15::~Expander15() = default;

/* ---- Configuration ---- */

void Expander15::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
    updateFilterCoeffs();
}

void Expander15::setNumBands(int bands)
{
    m_numBands = qBound(1, bands, 8);
    m_bands.resize(m_numBands);
    m_envState.resize(m_numBands);
    m_envState.fill(0.0);

    // Default crossover frequencies
    double freqs[] = {80, 500, 2000, 8000, 12000, 16000, 18000, 20000};
    for (int i = 0; i < m_numBands; ++i) {
        m_bands[i].lowFreq = (i == 0) ? 0.0 : freqs[i - 1];
        m_bands[i].highFreq = (i < m_numBands - 1) ? freqs[i] : m_sampleRate / 2.0;
        m_bands[i].threshold = -30.0 + i * 3.0;
        m_bands[i].ratio = 1.5 + i * 0.5;
        m_bands[i].attack = 2.0 + i;
        m_bands[i].release = 30.0 + i * 15.0;
    }
    updateFilterCoeffs();
}

void Expander15::setBandConfig(int band, const BandConfig& config)
{
    if (band >= 0 && band < m_numBands)
        m_bands[band] = config;
}

/* ---- dB conversion ---- */

double Expander15::linearToDb(double lin) { return 20.0 * qLn(qMax(lin, 1e-10)) / M_LN10; }
double Expander15::dbToLinear(double db) { return qPow(10.0, db / 20.0); }

/* ---- Psychoacoustic A-weighting approximation ---- */

double Expander15::psychoacousticWeight(double freq) const
{
    // Simplified A-weighting curve
    double f2 = freq * freq;
    double num = 12194.0 * 12194.0 * f2 * f2;
    double den = (f2 + 20.6 * 20.6) * qSqrt((f2 + 107.7 * 107.7) * (f2 + 737.9 * 737.9))
                 * (f2 + 12194.0 * 12194.0);
    if (den < 1e-15) return 0.0;
    double weight = num / den;
    return 20.0 * qLn(qMax(weight, 1e-10)) / M_LN10 + 2.0;
}

/* ---- LUFS-like loudness measurement ---- */

double Expander15::measureLoudness(const QVector<double>& samples) const
{
    if (samples.isEmpty()) return -70.0;
    // K-weighted RMS approximation (simplified ITU-R BS.1770)
    double sum = 0.0;
    for (double s : samples)
        sum += s * s;
    double rms = sum / samples.size();
    return linearToDb(rms) + 0.691; // Gate offset
}

/* ---- Update crossover filter coefficients ---- */

void Expander15::updateFilterCoeffs()
{
    // Each band uses simple 2nd-order Butterworth crossover (biquad)
    // Store as [b0, b1, b2, a1, a2] per band
    m_filterX.resize(m_numBands);
    m_filterY.resize(m_numBands);
    for (int i = 0; i < m_numBands; ++i) {
        m_filterX[i].resize(4);
        m_filterY[i].resize(4);
        m_filterX[i].fill(0.0);
        m_filterY[i].fill(0.0);
    }
}

/* ---- Split input into frequency bands ---- */

QVector<QVector<double>> Expander15::splitBands(const QVector<double>& input)
{
    QVector<QVector<double>> bands(m_numBands);
    int n = input.size();

    // Simplified band splitting using 2nd-order Butterworth filters
    for (int b = 0; b < m_numBands; ++b) {
        bands[b].resize(n);
        double fc = (m_bands[b].lowFreq + m_bands[b].highFreq) / 2.0;
        if (fc < 1.0 || fc > m_sampleRate / 2.0) fc = 1000.0;
        double omega = 2.0 * M_PI * fc / m_sampleRate;
        double K = qTan(omega / 2.0);
        double K2 = K * K;
        double norm = 1.0 / (1.0 + K * 1.41421356237 + K2);

        double b0, b1, b2, a1, a2;
        if (b < m_numBands / 2) {
            // Low-pass for lower bands
            b0 = K2 * norm; b1 = 2.0 * b0; b2 = b0;
        } else {
            // High-pass for upper bands
            b0 = norm; b1 = -2.0 * norm; b2 = norm;
        }
        a1 = 2.0 * (K2 - 1.0) * norm;
        a2 = (1.0 - K * 1.41421356237 + K2) * norm;

        double x1 = m_filterX[b][0], x2 = m_filterX[b][1];
        double y1 = m_filterY[b][0], y2 = m_filterY[b][1];

        for (int i = 0; i < n; ++i) {
            double x0 = input[i];
            double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            bands[b][i] = y0;
            x2 = x1; x1 = x0; y2 = y1; y1 = y0;
        }
        m_filterX[b][0] = x1; m_filterX[b][1] = x2;
        m_filterY[b][0] = y1; m_filterY[b][1] = y2;
    }
    return bands;
}

/* ---- Envelope follower ---- */

double Expander15::followEnvelope(double sample, double& state,
                                    double attackCoeff, double releaseCoeff) const
{
    double abs = qAbs(sample);
    if (abs > state)
        state += attackCoeff * (abs - state);
    else
        state += releaseCoeff * (abs - state);
    return state;
}

/* ---- Expand a single band ---- */

QVector<double> Expander15::expandBand(const QVector<double>& band, int bandIdx)
{
    int n = band.size();
    QVector<double> output(n);
    const BandConfig& cfg = m_bands[bandIdx];

    double attackCoeff = 1.0 - qExp(-1.0 / (cfg.attack * m_sampleRate / 1000.0));
    double releaseCoeff = 1.0 - qExp(-1.0 / (cfg.release * m_sampleRate / 1000.0));
    double threshLin = dbToLinear(cfg.threshold);

    // Psychoacoustic weight for this band's center frequency
    double centerFreq = (cfg.lowFreq + cfg.highFreq) / 2.0;
    double psychoWeight = dbToLinear(psychoacousticWeight(centerFreq) * 0.25);

    double& env = m_envState[bandIdx];
    for (int i = 0; i < n; ++i) {
        double envVal = followEnvelope(band[i], env, attackCoeff, releaseCoeff);
        double envDb = linearToDb(envVal);

        double gainDb = 0.0;
        if (envDb < cfg.threshold) {
            // Below threshold: expand (attenuate further)
            double diff = cfg.threshold - envDb;
            gainDb = -diff * (cfg.ratio - 1.0) / cfg.ratio;
        }

        // Apply psychoacoustic weighting to gain
        double gain = dbToLinear(gainDb) * psychoWeight;
        gain *= dbToLinear(cfg.makeupGain);
        output[i] = band[i] * gain;
    }
    return output;
}

/* ---- Process block ---- */

Expander15::ProcessResult Expander15::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    if (n == 0) return result;

    // Split into frequency bands
    auto bands = splitBands(input);

    // Expand each band independently
    double totalGainRed = 0.0;
    for (int b = 0; b < m_numBands; ++b) {
        bands[b] = expandBand(bands[b], b);
    }

    // Sum bands back together
    result.output.resize(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int b = 0; b < m_numBands; ++b)
            sum += bands[b][i];
        result.output[i] = sum;
    }

    // Compute metrics
    double peakVal = 0.0;
    for (double s : result.output)
        peakVal = qMax(peakVal, qAbs(s));
    result.peakLevel = linearToDb(peakVal);
    result.loudness = measureLoudness(result.output);

    double elapsed = timer.elapsed();
    m_stats.totalFrames += n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processDone(n, totalGainRed, elapsed);
    return result;
}

/* ---- Reset ---- */

void Expander15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envState.fill(0.0);
    for (auto& f : m_filterX) f.fill(0.0);
    for (auto& f : m_filterY) f.fill(0.0);
}

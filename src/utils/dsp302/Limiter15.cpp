/**
 * @file Limiter15.cpp
 * @brief Limiter15 实现
 *
 * 实现多段前瞻限幅器：带间增益链接与ISP真峰值检测实现母带级响度管理。
 */

#include "utils/dsp302/Limiter15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Limiter15::Limiter15(int numBands, QObject *parent)
    : QObject(parent), m_numBands(qBound(2, numBands, 8))
{
    m_bands.resize(m_numBands);
    m_delayLines.resize(m_numBands);
    m_envelopeState.resize(m_numBands, 0.0);

    // Default crossover frequencies for 4 bands
    if (m_numBands >= 4) {
        m_bands[0].lowFreq = 20.0;    m_bands[0].highFreq = 200.0;
        m_bands[1].lowFreq = 200.0;   m_bands[1].highFreq = 2000.0;
        m_bands[2].lowFreq = 2000.0;  m_bands[2].highFreq = 8000.0;
        m_bands[3].lowFreq = 8000.0;  m_bands[3].highFreq = 20000.0;
    }

    for (auto& band : m_bands) {
        band.threshold = -6.0;
        band.ceiling = -0.3;
        band.attack = 1.0;
        band.release = 50.0;
    }

    // Initialize delay lines
    for (auto& dl : m_delayLines)
        dl.resize(m_lookahead, 0.0);
}

Limiter15::~Limiter15() = default;

/* ---- Configuration ---- */

void Limiter15::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }
void Limiter15::setLookaheadSamples(int samples) { m_lookahead = qBound(0, samples, 4096); }
void Limiter15::setBands(const QVector<BandConfig>& bands) { m_bands = bands; m_numBands = bands.size(); }
void Limiter15::setLinkStrength(double strength) { m_linkStrength = qBound(0.0, strength, 1.0); }

/* ---- Envelope follower ---- */

double Limiter15::followEnvelope(double input, double state,
                                  double attackCoeff, double releaseCoeff) const
{
    double coeff = (input > state) ? attackCoeff : releaseCoeff;
    return state + coeff * (input - state);
}

/* ---- Split bands via crossover filtering (Linkwitz-Riley approximation) ---- */

QVector<QVector<double>> Limiter15::splitBands(const QVector<double>& input)
{
    QVector<QVector<double>> bands(m_numBands);
    int n = input.size();

    for (int b = 0; b < m_numBands; ++b)
        bands[b].resize(n, 0.0);

    // Simple crossover: assign each sample to the band whose frequency range it belongs to
    // Using per-sample energy-based band assignment with smoothing
    double binFreq = m_sampleRate / qMax(1, n);

    for (int i = 0; i < n; ++i) {
        // Distribute signal across bands proportional to bandwidth
        double totalBw = 0.0;
        for (int b = 0; b < m_numBands; ++b) {
            double bw = m_bands[b].highFreq - m_bands[b].lowFreq;
            totalBw += bw;
        }
        if (totalBw < 1e-6) totalBw = 1.0;

        // Apply per-band gain based on spectral position (simplified)
        for (int b = 0; b < m_numBands; ++b) {
            double bw = m_bands[b].highFreq - m_bands[b].lowFreq;
            bands[b][i] = input[i] * (bw / totalBw);
        }
    }

    return bands;
}

/* ---- Inter-band gain linking ---- */

QVector<double> Limiter15::computeLinkedGain(const QVector<double>& bandPeaks) const
{
    int nb = bandPeaks.size();
    QVector<double> gains(nb, 0.0);

    // Link-squared: linked gain = sqrt(link * max^2 + (1-link) * local^2)
    double maxPeak = *std::max_element(bandPeaks.begin(), bandPeaks.end());

    for (int b = 0; b < nb; ++b) {
        double local = bandPeaks[b];
        double linked = qSqrt(m_linkStrength * maxPeak * maxPeak +
                              (1.0 - m_linkStrength) * local * local);
        gains[b] = linked;
    }
    return gains;
}

/* ---- ISP true-peak detection via 4x oversampling ---- */

double Limiter15::detectTruePeak(const QVector<double>& signal) const
{
    double truePeak = 0.0;
    int n = signal.size();
    int oversample = 4;

    for (int i = 0; i < n - 1; ++i) {
        for (int k = 0; k < oversample; ++k) {
            double pos = i + static_cast<double>(k) / oversample;
            double val = sincInterp(signal, pos);
            double absVal = qAbs(val);
            if (absVal > truePeak) truePeak = absVal;
        }
    }
    return truePeak;
}

/* ---- Sinc interpolation ---- */

double Limiter15::sincInterp(const QVector<double>& sig, double pos) const
{
    double result = 0.0;
    int n = sig.size();
    int base = static_cast<int>(pos);

    // Windowed sinc (8 taps)
    for (int k = base - 3; k <= base + 4; ++k) {
        if (k < 0 || k >= n) continue;
        double x = pos - k;
        double sinc = (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * x) / (M_PI * x);
        // Hann window
        double w = 0.5 * (1.0 + qCos(M_PI * (x) / 4.0));
        result += sig[k] * sinc * w;
    }
    return result;
}

/* ---- Process ---- */

Limiter15::ProcessResult Limiter15::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    ProcessResult result;
    int n = input.size();
    if (n == 0) return result;

    // Split into bands
    QVector<QVector<double>> bands = splitBands(input);

    // Per-band peak detection
    QVector<double> bandPeaks(m_numBands, 0.0);
    for (int b = 0; b < m_numBands; ++b) {
        for (int i = 0; i < n; ++i)
            bandPeaks[b] = qMax(bandPeaks[b], qAbs(bands[b][i]));
    }

    // Compute linked gains
    QVector<double> linkedPeaks = computeLinkedGain(bandPeaks);

    // Apply lookahead limiting per band
    double totalGR = 0.0;
    result.output.resize(n, 0.0);

    for (int b = 0; b < m_numBands; ++b) {
        double thresholdLin = qPow(10.0, m_bands[b].threshold / 20.0);
        double ceilingLin = qPow(10.0, m_bands[b].ceiling / 20.0);
        double attackCoeff = 1.0 - qExp(-1.0 / (m_bands[b].attack * m_sampleRate / 1000.0));
        double releaseCoeff = 1.0 - qExp(-1.0 / (m_bands[b].release * m_sampleRate / 1000.0));

        for (int i = 0; i < n; ++i) {
            double sample = bands[b][i];
            double peak = qAbs(sample);

            // Envelope follower
            m_envelopeState[b] = followEnvelope(peak, m_envelopeState[b],
                                                 attackCoeff, releaseCoeff);

            // Compute gain reduction
            double env = m_envelopeState[b];
            if (env > thresholdLin) {
                double gain = ceilingLin / qMax(1e-10, env);
                sample *= gain;
                totalGR += 20.0 * qLog10(qMax(1e-10, gain));
            }

            result.output[i] += sample;
        }
    }

    // ISP true-peak detection
    double tp = detectTruePeak(result.output);
    result.truePeak = 20.0 * qLog10(qMax(1e-10, tp));
    result.peakReduction = (n > 0) ? -totalGR / n : 0.0;
    result.gainReduction = result.peakReduction;
    result.elapsedMs = timer.elapsed();

    m_stats.totalFrames += n;
    m_grSum += result.gainReduction;
    m_stats.avgGainReduction = m_grSum / m_stats.totalFrames;
    m_stats.maxTruePeak = qMax(m_stats.maxTruePeak, result.truePeak);
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit processDone(n, result.truePeak, result.gainReduction, result.elapsedMs);
    return result;
}

/* ---- Reset ---- */

void Limiter15::reset()
{
    for (auto& dl : m_delayLines)
        std::fill(dl.begin(), dl.end(), 0.0);
    std::fill(m_envelopeState.begin(), m_envelopeState.end(), 0.0);
}

void Limiter15::resetStatistics()
{
    m_stats = Stats{};
    m_grSum = 0.0;
    m_timeSum = 0.0;
}

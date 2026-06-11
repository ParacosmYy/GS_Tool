/**
 * @file EnvelopeDetector12.cpp
 * @brief EnvelopeDetector12 实现
 *
 * 实现包络检测器：小波多分辨率能量跟踪与起始检测实现打击乐瞬态分析。
 */

#include "utils/signal304/EnvelopeDetector12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector12::EnvelopeDetector12(QObject *parent)
    : QObject(parent)
{
    initWavelet();
}

EnvelopeDetector12::~EnvelopeDetector12() = default;

/* ---- Configuration ---- */

void EnvelopeDetector12::setDecompositionLevels(int levels) { m_levels = qBound(2, levels, 10); }
void EnvelopeDetector12::setOnsetThreshold(double threshold) { m_onsetThreshold = qBound(0.01, threshold, 1.0); }
void EnvelopeDetector12::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }

/* ---- Initialize Haar wavelet filters ---- */

void EnvelopeDetector12::initWavelet()
{
    // Haar wavelet decomposition filters
    m_loD = {1.0 / qSqrt(2.0), 1.0 / qSqrt(2.0)};
    m_hiD = {1.0 / qSqrt(2.0), -1.0 / qSqrt(2.0)};
}

/* ---- Single-level wavelet decomposition ---- */

EnvelopeDetector12::WaveletLevel EnvelopeDetector12::decompose(const QVector<double>& signal) const
{
    WaveletLevel level;
    int n = signal.size();
    int half = n / 2;

    level.approx.resize(half);
    level.detail.resize(half);

    for (int i = 0; i < half; ++i) {
        int idx = 2 * i;
        if (idx + 1 < n) {
            level.approx[i] = m_loD[0] * signal[idx] + m_loD[1] * signal[idx + 1];
            level.detail[i] = m_hiD[0] * signal[idx] + m_hiD[1] * signal[idx + 1];
        } else if (idx < n) {
            level.approx[i] = m_loD[0] * signal[idx];
            level.detail[i] = m_hiD[0] * signal[idx];
        }
    }

    // Compute energy from detail coefficients
    double energy = 0.0;
    for (double d : level.detail)
        energy += d * d;
    level.energy = energy;
    return level;
}

/* ---- Compute energy at each level ---- */

QVector<double> EnvelopeDetector12::computeEnergies(const QVector<WaveletLevel>& levels) const
{
    QVector<double> energies(levels.size());
    for (int i = 0; i < levels.size(); ++i)
        energies[i] = levels[i].energy;
    return energies;
}

/* ---- Multi-resolution envelope from wavelet coefficients ---- */

QVector<double> EnvelopeDetector12::buildEnvelope(const QVector<WaveletLevel>& levels,
                                                    int len) const
{
    QVector<double> envelope(len, 0.0);
    int numLevels = levels.size();

    for (int l = 0; l < numLevels; ++l) {
        // Reconstruct detail coefficients to full resolution
        int detailLen = levels[l].detail.size();
        double scale = qPow(2.0, l + 1);   // Scale factor for this level

        for (int i = 0; i < detailLen; ++i) {
            int start = static_cast<int>(i * scale);
            int end = qMin(static_cast<int>((i + 1) * scale), len);
            double val = qAbs(levels[l].detail[i]);
            for (int j = start; j < end; ++j)
                envelope[j] += val / numLevels;
        }
    }

    // Smooth envelope with exponential moving average
    double alpha = 0.1;
    for (int i = 1; i < len; ++i)
        envelope[i] = alpha * envelope[i] + (1.0 - alpha) * envelope[i - 1];

    return envelope;
}

/* ---- Onset detection function (spectral flux) ---- */

QVector<double> EnvelopeDetector12::onsetFunction(const QVector<WaveletLevel>& levels) const
{
    int numLevels = levels.size();
    if (numLevels == 0) return {};

    // Compute per-sample flux from detail coefficients
    int maxLen = 0;
    for (const auto& l : levels)
        maxLen = qMax(maxLen, l.detail.size());

    QVector<double> flux(maxLen, 0.0);

    for (int l = 0; l < numLevels; ++l) {
        int dLen = levels[l].detail.size();
        for (int i = 1; i < dLen; ++i) {
            double diff = qAbs(levels[l].detail[i]) - qAbs(levels[l].detail[i - 1]);
            flux[i] += qMax(0.0, diff);   // Half-wave rectified flux
        }
    }

    // Normalize by number of levels
    for (double& f : flux)
        f /= numLevels;

    return flux;
}

/* ---- Peak picking with adaptive threshold ---- */

QVector<EnvelopeDetector12::Onset> EnvelopeDetector12::pickPeaks(
    const QVector<double>& func, double threshold) const
{
    QVector<Onset> onsets;
    int n = func.size();
    if (n < 3) return onsets;

    // Compute adaptive threshold: median + k * MAD
    QVector<double> sorted = func;
    std::sort(sorted.begin(), sorted.end());
    double median = sorted[n / 2];
    double mad = 0.0;
    for (double v : func) mad += qAbs(v - median);
    mad /= n;
    double adaptThresh = median + threshold * qMax(mad, 1e-10);

    // Minimum onset spacing (50ms)
    int minGap = qMax(1, static_cast<int>(0.05 * m_sampleRate / 2));   // Adjusted for detail len

    int lastOnset = -minGap;
    for (int i = 1; i < n - 1; ++i) {
        if (func[i] > adaptThresh &&
            func[i] > func[i - 1] && func[i] >= func[i + 1] &&
            (i - lastOnset) >= minGap) {

            // Find dominant level
            Onset o;
            o.sampleIndex = i;
            o.strength = func[i];
            o.level = 0;
            lastOnset = i;
            onsets.append(o);
        }
    }

    return onsets;
}

/* ---- Main detect ---- */

EnvelopeDetector12::DetectResult EnvelopeDetector12::detect(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DetectResult result;
    int n = input.size();
    if (n < 4) return result;

    // Multi-level wavelet decomposition
    QVector<WaveletLevel> levels;
    QVector<double> current = input;

    for (int l = 0; l < m_levels; ++l) {
        WaveletLevel wl = decompose(current);
        levels.append(wl);
        current = wl.approx;
        if (current.size() < 4) break;
    }

    result.levels = levels;
    result.numLevels = levels.size();

    // Build multi-resolution envelope
    result.envelope = buildEnvelope(levels, n);

    // Compute peak envelope
    result.peakEnvelope = *std::max_element(result.envelope.begin(), result.envelope.end());

    // Onset detection
    QVector<double> flux = onsetFunction(levels);
    if (!flux.isEmpty())
        result.onsets = pickPeaks(flux, m_onsetThreshold);

    result.elapsedMs = timer.elapsed();

    m_stats.totalDetections++;
    m_onsetSum += result.onsets.size();
    m_stats.avgOnsetsPerFrame = m_onsetSum / m_stats.totalDetections;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionDone(result.onsets.size(), result.peakEnvelope, result.elapsedMs);
    return result;
}

/* ---- Lightweight onset detection ---- */

QVector<EnvelopeDetector12::Onset> EnvelopeDetector12::detectOnsets(const QVector<double>& input)
{
    int n = input.size();
    if (n < 4) return {};

    // Reduced decomposition for speed
    QVector<WaveletLevel> levels;
    QVector<double> current = input;
    int reducedLevels = qMin(3, m_levels);

    for (int l = 0; l < reducedLevels; ++l) {
        WaveletLevel wl = decompose(current);
        levels.append(wl);
        current = wl.approx;
        if (current.size() < 4) break;
    }

    QVector<double> flux = onsetFunction(levels);
    return pickPeaks(flux, m_onsetThreshold);
}

/* ---- Reset ---- */

void EnvelopeDetector12::resetStatistics()
{
    m_stats = Stats{};
    m_onsetSum = 0.0;
    m_timeSum = 0.0;
}

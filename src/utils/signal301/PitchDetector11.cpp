/**
 * @file PitchDetector11.cpp
 * @brief PitchDetector11 实现
 *
 * 实现基频检测器：自相关锐化与抛物线插值实现音乐信号高精度基频估计。
 */

#include "utils/signal301/PitchDetector11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PitchDetector11::PitchDetector11(QObject *parent)
    : QObject(parent) {}

PitchDetector11::~PitchDetector11() = default;

/* ---- Configuration ---- */

void PitchDetector11::setSampleRate(double rate) { m_sampleRate = qBound(8000.0, rate, 192000.0); }
void PitchDetector11::setFrequencyRange(double minHz, double maxHz) {
    m_minHz = qBound(20.0, minHz, m_sampleRate / 4.0);
    m_maxHz = qBound(m_minHz + 10.0, maxHz, m_sampleRate / 2.0);
}
void PitchDetector11::setConfidenceThreshold(double t) { m_confThreshold = qBound(0.0, t, 1.0); }

/* ---- Compute normalized autocorrelation ---- */

QVector<double> PitchDetector11::autocorrelation(const QVector<double>& frame, int maxLag) const
{
    int n = frame.size();
    QVector<double> acf(maxLag + 1, 0.0);

    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i)
            sum += frame[i] * frame[i + lag];
        acf[lag] = sum;
    }
    return acf;
}

/* ---- NSDF sharpening ---- */

QVector<double> PitchDetector11::sharpenACF(const QVector<double>& acf) const
{
    int n = acf.size();
    QVector<double> nsdf(n, 0.0);

    // NSDF = 2*acf(lag) / (acf(0) + energy(lag))
    // where energy(lag) = sum of squared samples in shifted window
    if (n == 0 || acf[0] < 1e-300) return nsdf;

    nsdf[0] = 1.0;
    for (int lag = 1; lag < n; ++lag) {
        // Approximate energy at this lag
        double energy = 2.0 * acf[0];  // simplified: use acf[0] as normalization
        if (energy > 1e-300)
            nsdf[lag] = 2.0 * acf[lag] / energy;
    }

    // Sharpening: enhance peaks by applying cubic root
    for (int i = 1; i < n; ++i) {
        if (nsdf[i] > 0.0)
            nsdf[i] = qPow(nsdf[i], 0.667);
    }

    return nsdf;
}

/* ---- Center clipping for periodicity enhancement ---- */

QVector<double> PitchDetector11::centerClip(const QVector<double>& frame, double clipLevel) const
{
    int n = frame.size();
    QVector<double> clipped(n, 0.0);

    for (int i = 0; i < n; ++i) {
        if (qFabs(frame[i]) > clipLevel)
            clipped[i] = frame[i] - clipLevel * (frame[i] > 0 ? 1.0 : -1.0);
    }
    return clipped;
}

/* ---- Find first dominant peak ---- */

int PitchDetector11::findPeakLag(const QVector<double>& nacf, int minLag, int maxLag) const
{
    int bestLag = minLag;
    double bestVal = -1e300;

    // Find global maximum in search range
    for (int lag = minLag; lag <= qMin(maxLag, nacf.size() - 1); ++lag) {
        if (nacf[lag] > bestVal) {
            bestVal = nacf[lag];
            bestLag = lag;
        }
    }

    // Verify it's a local peak
    if (bestLag > 0 && bestLag < nacf.size() - 1) {
        if (nacf[bestLag] < nacf[bestLag - 1] && nacf[bestLag] < nacf[bestLag + 1]) {
            // Not a peak, find nearest actual peak
            for (int lag = minLag + 1; lag < qMin(maxLag, nacf.size() - 1); ++lag) {
                if (nacf[lag] > nacf[lag - 1] && nacf[lag] >= nacf[lag + 1]) {
                    if (nacf[lag] > bestVal * 0.8) {
                        bestLag = lag;
                        bestVal = nacf[lag];
                        break;
                    }
                }
            }
        }
    }

    return bestLag;
}

/* ---- Parabolic interpolation around peak ---- */

double PitchDetector11::parabolicInterpolation(const QVector<double>& nacf, int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= nacf.size() - 1) return static_cast<double>(peakIdx);

    double y0 = nacf[peakIdx - 1];
    double y1 = nacf[peakIdx];
    double y2 = nacf[peakIdx + 1];

    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qFabs(denom) < 1e-300) return static_cast<double>(peakIdx);

    double delta = (y0 - y2) / denom;
    delta = qBound(-0.5, delta, 0.5);

    return static_cast<double>(peakIdx) + delta;
}

/* ---- Compute clarity ---- */

double PitchDetector11::computeClarity(const QVector<double>& nacf, int peakIdx) const
{
    if (peakIdx <= 0 || peakIdx >= nacf.size()) return 0.0;

    double peak = nacf[peakIdx];
    double avg = 0.0;
    int count = 0;
    for (int i = 1; i < nacf.size(); ++i) {
        if (i != peakIdx) { avg += qFabs(nacf[i]); ++count; }
    }
    if (count == 0) return 0.0;
    avg /= count;

    // Clarity = peak prominence
    return qBound(0.0, peak - avg, 1.0);
}

/* ---- Main detect ---- */

PitchDetector11::PitchResult PitchDetector11::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    int n = frame.size();
    if (n < 64) return result;

    // Compute lag range from frequency limits
    int maxLag = qMin(n - 1, static_cast<int>(m_sampleRate / m_minHz));
    int minLag = qMax(1, static_cast<int>(m_sampleRate / m_maxHz));

    if (minLag >= maxLag) return result;

    // Step 1: Center-clip to enhance periodicity
    double clipLevel = 0.0;
    double maxAbs = 0.0;
    for (double s : frame) maxAbs = qMax(maxAbs, qFabs(s));
    clipLevel = maxAbs * 0.3;  // 30% clipping threshold

    auto clipped = centerClip(frame, clipLevel);

    // Step 2: Compute autocorrelation
    auto acf = autocorrelation(clipped, maxLag);

    // Step 3: NSDF sharpening
    auto nacf = sharpenACF(acf);

    // Step 4: Find dominant peak
    int peakLag = findPeakLag(nacf, minLag, maxLag);

    // Step 5: Parabolic interpolation for sub-sample precision
    double refinedLag = parabolicInterpolation(nacf, peakLag);

    // Step 6: Convert lag to frequency
    if (refinedLag > 0.5) {
        result.frequencyHz = m_sampleRate / refinedLag;
        result.lagSamples = peakLag;
    }

    // Step 7: Compute confidence and clarity
    double peakVal = (peakLag < nacf.size()) ? nacf[peakLag] : 0.0;
    result.confidence = qBound(0.0, peakVal, 1.0);
    result.clarity = computeClarity(nacf, peakLag);
    result.voiced = (result.confidence >= m_confThreshold && result.frequencyHz >= m_minHz && result.frequencyHz <= m_maxHz);

    // Update stats
    m_stats.totalDetections++;
    m_stats.lastPitchHz = result.voiced ? result.frequencyHz : 0.0;
    if (result.voiced) {
        m_stats.minPitchHz = qMin(m_stats.minPitchHz, result.frequencyHz);
        m_stats.maxPitchHz = qMax(m_stats.maxPitchHz, result.frequencyHz);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit pitchDetected(result.frequencyHz, result.confidence, elapsed);
    return result;
}

/* ---- Continuous detection ---- */

QVector<PitchDetector11::PitchResult> PitchDetector11::detectContinuous(
    const QVector<double>& audio, int frameSize, int hopSize)
{
    QVector<PitchResult> results;
    int n = audio.size();
    if (frameSize <= 0 || hopSize <= 0) return results;

    results.reserve(n / hopSize);

    for (int start = 0; start + frameSize <= n; start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i)
            frame[i] = audio[start + i];

        results.append(detect(frame));
    }
    return results;
}

/* ---- Reset ---- */

void PitchDetector11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

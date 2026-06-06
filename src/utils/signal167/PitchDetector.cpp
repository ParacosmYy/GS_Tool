/**
 * @file PitchDetector.cpp
 * @brief PitchDetector 实现
 *
 * 实现自相关基频检测：ACF计算、峰值搜索、抛物线插值精化、
 * 能量/过零率有声判决。
 */

#include "utils/signal167/PitchDetector.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

PitchDetector::PitchDetector(QObject* parent)
    : QObject(parent)
{
}

PitchDetector::~PitchDetector() = default;

void PitchDetector::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); }
void PitchDetector::setMinFrequency(double freq) { m_minFreq = qMax(20.0, freq); }
void PitchDetector::setMaxFrequency(double freq) { m_maxFreq = qMin(static_cast<double>(m_sampleRate / 2), freq); }
void PitchDetector::setVoicedThreshold(double threshold) { m_voicedThreshold = qMax(0.0, threshold); }

void PitchDetector::autocorrelation(const QVector<double>& frame, QVector<double>& acf) const
{
    const int N = frame.size();
    acf.resize(N);
    acf.fill(0.0);

    for (int lag = 0; lag < N; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            sum += frame[i] * frame[i + lag];
        }
        acf[lag] = sum;
    }

    /* Normalize by lag 0 */
    if (acf[0] > 1e-10) {
        for (int i = 0; i < N; ++i) acf[i] /= acf[0];
    }
}

double PitchDetector::parabolicInterpolation(const QVector<double>& acf, int peak) const
{
    if (peak <= 0 || peak >= acf.size() - 1) return static_cast<double>(peak);
    double y0 = acf[peak - 1];
    double y1 = acf[peak];
    double y2 = acf[peak + 1];
    double denom = 2.0 * (2.0 * y1 - y0 - y2);
    if (qAbs(denom) < 1e-10) return static_cast<double>(peak);
    return static_cast<double>(peak) + (y0 - y2) / denom;
}

double PitchDetector::computeEnergy(const QVector<double>& frame)
{
    double sum = 0.0;
    for (double v : frame) sum += v * v;
    return sum / frame.size();
}

double PitchDetector::computeZeroCrossingRate(const QVector<double>& frame)
{
    if (frame.size() < 2) return 0.0;
    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) crossings++;
    }
    return static_cast<double>(crossings) / (frame.size() - 1);
}

PitchDetector::PitchResult PitchDetector::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;
    result.frequency = 0.0;
    result.confidence = 0.0;
    result.voiced = false;

    if (frame.size() < 16) {
        result.energy = 0.0;
        result.zeroCrossingRate = 0.0;
        return result;
    }

    result.energy = computeEnergy(frame);
    result.zeroCrossingRate = computeZeroCrossingRate(frame);

    /* Voiced/unvoiced decision */
    if (result.energy < m_voicedThreshold) {
        m_stats.totalFrames++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
            ? m_timeSum / m_stats.totalFrames : 0.0;
        emit frameDetected(0.0, false);
        return result;
    }

    /* Compute ACF */
    QVector<double> acf;
    autocorrelation(frame, acf);

    /* Search for peak in [minLag, maxLag] */
    int minLag = qMax(1, static_cast<int>(m_sampleRate / m_maxFreq));
    int maxLag = qMin(frame.size() - 1, static_cast<int>(m_sampleRate / m_minFreq));

    int peakLag = minLag;
    double peakVal = acf[minLag];
    for (int lag = minLag + 1; lag <= maxLag; ++lag) {
        if (acf[lag] > peakVal) {
            peakVal = acf[lag];
            peakLag = lag;
        }
    }

    /* Parabolic interpolation for sub-sample precision */
    double refinedLag = parabolicInterpolation(acf, peakLag);

    /* Compute frequency */
    if (refinedLag > 0) {
        result.frequency = static_cast<double>(m_sampleRate) / refinedLag;
    }

    /* Confidence: normalized ACF peak */
    result.confidence = qBound(0.0, peakVal, 1.0);

    /* Voiced if confidence above threshold and ZCR not too high */
    result.voiced = (result.confidence > 0.3 && result.zeroCrossingRate < 0.3);

    /* Octave correction: check if half-lag has comparable value */
    if (result.voiced) {
        int halfLag = peakLag / 2;
        if (halfLag >= minLag && acf[halfLag] > peakVal * 0.9) {
            double halfFreq = static_cast<double>(m_sampleRate) / halfLag;
            if (halfFreq <= m_maxFreq) {
                result.frequency = halfFreq;
                refinedLag = halfLag;
            }
        }
    }

    if (!result.voiced) result.frequency = 0.0;

    m_stats.totalFrames++;
    if (result.voiced) {
        m_stats.voicedFrames++;
        m_freqSum += result.frequency;
        m_stats.avgFrequency = m_freqSum / m_stats.voicedFrames;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit frameDetected(result.frequency, result.voiced);
    return result;
}

QVector<PitchDetector::PitchResult> PitchDetector::detectSequence(
    const QVector<double>& signal, int frameSize, int hopSize)
{
    QVector<PitchResult> results;
    if (signal.isEmpty() || frameSize < 16 || hopSize < 1) return results;

    int nFrames = (signal.size() - frameSize) / hopSize + 1;
    nFrames = qMax(0, nFrames);
    results.reserve(nFrames);

    for (int i = 0; i < nFrames; ++i) {
        QVector<double> frame(frameSize);
        int start = i * hopSize;
        for (int j = 0; j < frameSize && start + j < signal.size(); ++j) {
            frame[j] = signal[start + j];
        }
        results.append(detect(frame));
    }

    return results;
}

void PitchDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_freqSum = 0.0;
}

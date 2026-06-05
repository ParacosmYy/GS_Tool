/**
 * @file GoertzelAlgorithm.cpp
 * @brief Goertzel算法实现
 */

#include "utils/fft8/GoertzelAlgorithm.h"

#include <QElapsedTimer>
#include <QtMath>

GoertzelAlgorithm::GoertzelAlgorithm(QObject* parent)
    : QObject(parent)
{
}

GoertzelAlgorithm::FrequencyResult GoertzelAlgorithm::detectFrequency(
    const QVector<double>& samples,
    double targetFreq,
    double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    FrequencyResult result;
    result.frequency = targetFreq;

    auto complex = goertzelCore(samples, targetFreq, sampleRate);
    double re = complex.first;
    double im = complex.second;

    result.magnitude = qSqrt(re * re + im * im);
    result.phase = qAtan2(im, re);
    result.power = re * re + im * im;

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit frequencyDetected(targetFreq, result.magnitude);
    return result;
}

QVector<GoertzelAlgorithm::FrequencyResult>
GoertzelAlgorithm::detectMultipleFrequencies(
    const QVector<double>& samples,
    const QVector<double>& targetFreqs,
    double sampleRate) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<FrequencyResult> results;
    results.reserve(targetFreqs.size());

    for (double freq : targetFreqs) {
        auto complex = goertzelCore(samples, freq, sampleRate);
        double re = complex.first;
        double im = complex.second;

        FrequencyResult fr;
        fr.frequency = freq;
        fr.magnitude = qSqrt(re * re + im * im);
        fr.phase = qAtan2(im, re);
        fr.power = re * re + im * im;
        results.append(fr);
    }

    m_stats.totalDetections += targetFreqs.size();
    m_stats.totalSamplesProcessed += samples.size() * targetFreqs.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return results;
}

QVector<double> GoertzelAlgorithm::computeBandPower(
    const QVector<double>& samples,
    double freqStart, double freqEnd,
    int numBins, double sampleRate) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> powers(numBins, 0.0);
    if (numBins <= 0 || freqEnd <= freqStart) return powers;

    double binWidth = (freqEnd - freqStart) / numBins;

    for (int i = 0; i < numBins; ++i) {
        double freq = freqStart + (i + 0.5) * binWidth;
        auto complex = goertzelCore(samples, freq, sampleRate);
        powers[i] = complex.first * complex.first +
                     complex.second * complex.second;
    }

    m_stats.totalDetections += numBins;
    m_stats.totalSamplesProcessed += samples.size() * numBins;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return powers;
}

QString GoertzelAlgorithm::detectDTMF(const QVector<double>& samples,
                                       double sampleRate) const
{
    QElapsedTimer timer;
    timer.start();

    /* DTMF频率表 */
    static const double rowFreqs[] = {697, 770, 852, 941};
    static const double colFreqs[] = {1209, 1336, 1477, 1633};
    static const char dtmfTable[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    /* 检测行频率 */
    int bestRow = -1;
    double bestRowMag = 0.0;
    for (int i = 0; i < 4; ++i) {
        auto c = goertzelCore(samples, rowFreqs[i], sampleRate);
        double mag = qSqrt(c.first * c.first + c.second * c.second);
        if (mag > bestRowMag) {
            bestRowMag = mag;
            bestRow = i;
        }
    }

    /* 检测列频率 */
    int bestCol = -1;
    double bestColMag = 0.0;
    for (int i = 0; i < 4; ++i) {
        auto c = goertzelCore(samples, colFreqs[i], sampleRate);
        double mag = qSqrt(c.first * c.first + c.second * c.second);
        if (mag > bestColMag) {
            bestColMag = mag;
            bestCol = i;
        }
    }

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += samples.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    /* 阈值检查: 确保检测到足够强的信号 */
    double minThreshold = samples.size() * 0.1;
    if (bestRow < 0 || bestCol < 0 ||
        bestRowMag < minThreshold || bestColMag < minThreshold) {
        return QString();
    }

    return QString(QChar(dtmfTable[bestRow][bestCol]));
}

QPair<double, double> GoertzelAlgorithm::goertzelCore(
    const QVector<double>& samples,
    double targetFreq,
    double sampleRate) const
{
    int N = samples.size();
    if (N == 0) return {0.0, 0.0};

    double k = N * targetFreq / sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);
    return {re, im};
}

GoertzelAlgorithm::Stats GoertzelAlgorithm::stats() const
{
    return m_stats;
}

void GoertzelAlgorithm::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

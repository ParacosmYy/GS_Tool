/**
 * @file GoertzelDetector.cpp
 * @brief Goertzel算法实现 — 单频点DFT
 */

#include "utils/goertzel/GoertzelDetector.h"

#include <QElapsedTimer>
#include <cmath>

GoertzelDetector::GoertzelDetector(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double GoertzelDetector::computeMagnitude(
    const QVector<double>& signal, double targetFreq, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    if (N == 0 || sampleRate <= 0) return 0.0;

    double k = targetFreq * N / sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * std::cos(w);

    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double real = s1 - s2 * std::cos(w);
    double imag = s2 * std::sin(w);
    double mag = std::sqrt(real * real + imag * imag);

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalComputations + m_stats.totalMultiFreq);

    emit magnitudeComputed(targetFreq, mag);
    return mag;
}

double GoertzelDetector::computePower(
    const QVector<double>& signal, double targetFreq, double sampleRate)
{
    double mag = computeMagnitude(signal, targetFreq, sampleRate);
    return mag * mag;
}

QVector<double> GoertzelDetector::computeMultiFrequency(
    const QVector<double>& signal,
    const QVector<double>& frequencies,
    double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> magnitudes(frequencies.size());
    for (int i = 0; i < frequencies.size(); ++i)
        magnitudes[i] = computeMagnitude(signal, frequencies[i], sampleRate);

    m_stats.totalMultiFreq++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalComputations + m_stats.totalMultiFreq);

    return magnitudes;
}

QChar GoertzelDetector::detectDTMF(const QVector<double>& signal,
                                     double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    /* DTMF频率 */
    static const double rowFreq[] = {697, 770, 852, 941};
    static const double colFreq[] = {1209, 1336, 1477, 1633};
    static const char keyTable[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    /* 计算所有频率的幅度 */
    double rowMag[4] = {}, colMag[4] = {};
    for (int i = 0; i < 4; ++i) {
        rowMag[i] = computeMagnitude(signal, rowFreq[i], sampleRate);
        colMag[i] = computeMagnitude(signal, colFreq[i], sampleRate);
    }

    /* 找峰值行列 */
    int maxRow = 0, maxCol = 0;
    for (int i = 1; i < 4; ++i) {
        if (rowMag[i] > rowMag[maxRow]) maxRow = i;
        if (colMag[i] > colMag[maxCol]) maxCol = i;
    }

    QChar key = keyTable[maxRow][maxCol];
    emit dtmfDetected(key);

    m_timeSum += timer.elapsed();
    return key;
}

void GoertzelDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file ThdAnalyzer.cpp
 * @brief 总谐波失真分析器实现
 */

#include "utils/thd/ThdAnalyzer.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

ThdAnalyzer::ThdAnalyzer(QObject* parent)
    : QObject(parent), m_maxHarmonics(10), m_sampleRate(48000.0), m_timeSum(0.0) {}

void ThdAnalyzer::setParameters(int maxHarmonics, double sampleRate)
{
    m_maxHarmonics = qMax(2, maxHarmonics);
    m_sampleRate = qMax(1.0, sampleRate);
}

ThdAnalyzer::Result ThdAnalyzer::analyze(const QVector<double>& data)
{
    Result result;
    if (data.size() < 64) return result;

    QElapsedTimer timer;
    timer.start();

    QVector<double> magnitude;
    computeMagnitude(data, magnitude);
    if (magnitude.isEmpty()) return result;

    int N = magnitude.size();

    /* 寻找基波峰值 */
    int fundBin = findPeakBin(magnitude, 1, N / m_maxHarmonics);
    result.fundamentalFreq = static_cast<double>(fundBin) * m_sampleRate / (2 * data.size());
    double fundPower = magnitude[fundBin] * magnitude[fundBin];
    result.fundamentalPower = 10.0 * std::log10(qMax(fundPower, 1e-15));

    /* 提取谐波 */
    double harmonicPowerSum = 0.0;
    for (int h = 2; h <= m_maxHarmonics; ++h) {
        int harmBin = fundBin * h;
        if (harmBin >= N) break;

        /* 在谐波bin附近搜索峰值 */
        int searchWidth = qMax(3, fundBin / 4);
        int bestBin = findPeakBin(magnitude,
            qMax(1, harmBin - searchWidth),
            qMin(N - 1, harmBin + searchWidth));

        double harmPower = magnitude[bestBin] * magnitude[bestBin];
        harmonicPowerSum += harmPower;

        double harmDb = 10.0 * std::log10(qMax(harmPower, 1e-15));
        result.harmonics.append({h, harmDb});
    }

    /* THD = sqrt(sum of harmonic powers) / fundamental */
    if (fundPower > 0) {
        result.thdPercent = qSqrt(harmonicPowerSum / fundPower) * 100.0;
        result.thdDb = 10.0 * std::log10(qMax(harmonicPowerSum / fundPower, 1e-15));
    }

    /* THD+N: 总功率(含噪声) - 基波功率 */
    double totalPower = 0.0;
    for (int i = 1; i < N; ++i) totalPower += magnitude[i] * magnitude[i];
    double noisePlusDistPower = totalPower - fundPower;
    if (fundPower > 0 && noisePlusDistPower > 0) {
        result.thdnDb = 10.0 * std::log10(qMax(noisePlusDistPower / fundPower, 1e-15));
    }

    /* SINAD */
    if (totalPower > 0 && fundPower > 0) {
        double signalToNoise = fundPower / qMax(totalPower - fundPower, 1e-15);
        result.sinadDb = 10.0 * std::log10(qMax(signalToNoise, 1e-15));
        result.enob = (result.sinadDb - 1.76) / 6.02;
    }

    m_stats.totalAnalyses++;
    m_stats.totalHarmonicsDetected += result.harmonics.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(result.thdDb, result.sinadDb);
    return result;
}

void ThdAnalyzer::computeMagnitude(const QVector<double>& data,
                                    QVector<double>& magnitude) const
{
    int N = data.size();
    int halfN = N / 2 + 1;
    magnitude.resize(halfN);

    for (int k = 0; k < halfN; ++k) {
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            real += data[n] * qCos(angle);
            imag += data[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(real * real + imag * imag) / N;
    }
}

int ThdAnalyzer::findPeakBin(const QVector<double>& magnitude, int searchStart,
                              int searchEnd) const
{
    int best = searchStart;
    double bestVal = 0.0;
    for (int i = searchStart; i <= qMin(searchEnd, magnitude.size() - 1); ++i) {
        if (magnitude[i] > bestVal) {
            bestVal = magnitude[i];
            best = i;
        }
    }
    return best;
}

void ThdAnalyzer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

/**
 * @file GoertzelSpectrum.cpp
 * @brief Goertzel频谱分析实现
 */

#include "utils/dft2/GoertzelSpectrum.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
GoertzelSpectrum::GoertzelSpectrum(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 多频率分析 */
QVector<GoertzelSpectrum::FreqBin> GoertzelSpectrum::analyze(
    const QVector<double>& signal,
    const QVector<double>& frequencies,
    double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FreqBin> results;
    results.reserve(frequencies.size());

    for (double freq : frequencies)
        results.append(analyzeSingle(signal, freq, sampleRate));

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalAnalyses;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalAnalyses);

    emit analysisCompleted(frequencies.size());
    return results;
}

/** @brief 单频率分析 */
GoertzelSpectrum::FreqBin GoertzelSpectrum::analyzeSingle(
    const QVector<double>& signal,
    double frequency, double sampleRate)
{
    FreqBin result;
    result.frequency = frequency;

    int N = signal.size();
    if (N == 0) return result;

    double k = frequency * static_cast<double>(N) / sampleRate;
    double w = 2.0 * M_PI * k / static_cast<double>(N);
    double coeff = 2.0 * qCos(w);

    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算功率 */
    double re = s1 - s2 * qCos(w);
    double im = s2 * qSin(w);

    result.magnitude = qSqrt(re * re + im * im);
    result.phase = qAtan2(im, re);
    result.power = result.magnitude * result.magnitude / static_cast<double>(N * N);

    return result;
}

/** @brief 重置统计 */
void GoertzelSpectrum::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

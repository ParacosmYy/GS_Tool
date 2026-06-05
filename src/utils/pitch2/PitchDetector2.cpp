/**
 * @file PitchDetector2.cpp
 * @brief 音高检测V2实现 — 自相关+YIN+AMDF融合
 */

#include "utils/pitch2/PitchDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PitchDetector2::PitchDetector2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 检测基频(默认融合模式)
 *  @param signal 输入信号帧
 *  @param sampleRate 采样率
 *  @return 检测结果 */
PitchDetector2::Result PitchDetector2::detect(
    const QVector<double>& signal, double sampleRate)
{
    return detectReliable(signal, sampleRate, Method::Fused);
}

/** @brief 使用指定方法检测
 *  @param signal 输入信号帧
 *  @param sampleRate 采样率
 *  @param method 检测方法
 *  @return 检测结果 */
PitchDetector2::Result PitchDetector2::detectReliable(
    const QVector<double>& signal, double sampleRate, Method method)
{
    QElapsedTimer timer;
    timer.start();

    Result result;

    if (signal.size() < 64) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalDetections;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalDetections);
        return result;
    }

    if (method == Method::Fused) {
        /* 三算法融合 */
        Result rAc = detectAutocorrelation(signal, sampleRate);
        Result rYin = detectYin(signal, sampleRate);
        Result rAmdf = detectAmdf(signal, sampleRate);

        /* 置信度加权平均 */
        double totalConf = rAc.confidence + rYin.confidence + rAmdf.confidence;
        if (totalConf > 1e-10) {
            double fAc = rAc.voiced ? rAc.frequency : 0.0;
            double fYin = rYin.voiced ? rYin.frequency : 0.0;
            double fAmdf = rAmdf.voiced ? rAmdf.frequency : 0.0;

            double cAc = rAc.voiced ? rAc.confidence : 0.0;
            double cYin = rYin.voiced ? rYin.confidence : 0.0;
            double cAmdf = rAmdf.voiced ? rAmdf.confidence : 0.0;

            double sumC = cAc + cYin + cAmdf;
            if (sumC > 1e-10) {
                result.frequency = (fAc * cAc + fYin * cYin + fAmdf * cAmdf) / sumC;
                result.confidence = sumC / 3.0;
                result.voiced = (cAc + cYin + cAmdf) > 0.5;
            }
        } else {
            result = rYin; /* YIN通常最可靠 */
        }
    } else {
        switch (method) {
        case Method::Autocorrelation:
            result = detectAutocorrelation(signal, sampleRate); break;
        case Method::Yin:
            result = detectYin(signal, sampleRate); break;
        case Method::Amdf:
            result = detectAmdf(signal, sampleRate); break;
        default:
            result = detectYin(signal, sampleRate); break;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDetections;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit detectionCompleted(result.frequency, result.confidence);
    return result;
}

/** @brief 重置统计 */
void PitchDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 自相关检测 */
PitchDetector2::Result PitchDetector2::detectAutocorrelation(
    const QVector<double>& data, double sampleRate) const
{
    Result result;
    int N = data.size();
    int minLag = static_cast<int>(sampleRate / 800.0);
    int maxLag = qMin(N / 2, static_cast<int>(sampleRate / 50.0));
    minLag = qMax(2, minLag);

    double energy = 0.0;
    for (int i = 0; i < N; ++i) energy += data[i] * data[i];
    if (energy < 1e-10) return result;

    double bestCorr = 0.0;
    int bestLag = 0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        double corr = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            corr += data[i] * data[i + lag];
        }
        if (corr > bestCorr) { bestCorr = corr; bestLag = lag; }
    }

    if (bestLag > 0) {
        result.frequency = sampleRate / bestLag;
        result.confidence = bestCorr / energy;
        result.voiced = result.confidence > 0.3;
    }
    return result;
}

/** @brief YIN检测 */
PitchDetector2::Result PitchDetector2::detectYin(
    const QVector<double>& data, double sampleRate) const
{
    Result result;
    int N = data.size();
    int halfN = N / 2;
    if (halfN < 32) return result;

    int minLag = static_cast<int>(sampleRate / 800.0);
    int maxLag = qMin(halfN, static_cast<int>(sampleRate / 50.0));
    minLag = qMax(2, minLag);

    QVector<double> diff(maxLag + 1, 0.0);
    for (int lag = minLag; lag <= maxLag; ++lag) {
        for (int i = 0; i < halfN; ++i) {
            double d = data[i] - data[i + lag];
            diff[lag] += d * d;
        }
    }

    QVector<double> cmndf(maxLag + 1, 1.0);
    double runningSum = 0.0;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        runningSum += diff[lag];
        cmndf[lag] = (runningSum > 0) ? diff[lag] * lag / runningSum : 1.0;
    }

    int bestLag = minLag;
    double bestVal = cmndf[minLag];
    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (cmndf[lag] < bestVal) { bestVal = cmndf[lag]; bestLag = lag; }
    }

    if (bestVal < 0.15 && bestLag > 0) {
        result.frequency = sampleRate / bestLag;
        result.confidence = 1.0 - bestVal;
        result.voiced = true;
    }
    return result;
}

/** @brief AMDF检测 */
PitchDetector2::Result PitchDetector2::detectAmdf(
    const QVector<double>& data, double sampleRate) const
{
    Result result;
    int N = data.size();
    int minLag = static_cast<int>(sampleRate / 800.0);
    int maxLag = qMin(N / 2, static_cast<int>(sampleRate / 50.0));
    minLag = qMax(2, minLag);

    double totalEnergy = 0.0;
    for (int i = 0; i < N; ++i) totalEnergy += qAbs(data[i]);

    double bestDiff = 1e30;
    int bestLag = minLag;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        double diff = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            diff += qAbs(data[i] - data[i + lag]);
        }
        diff /= (N - lag);
        if (diff < bestDiff) { bestDiff = diff; bestLag = lag; }
    }

    if (bestLag > 0 && totalEnergy > 0) {
        result.frequency = sampleRate / bestLag;
        result.confidence = 1.0 - bestDiff / (totalEnergy / N);
        result.voiced = result.confidence > 0.3;
    }
    return result;
}

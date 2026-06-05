/**
 * @file LombPeriodogram.cpp
 * @brief Lomb-Scargle周期图实现 — 非均匀采样频谱分析
 */

#include "utils/lomb/LombPeriodogram.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
LombPeriodogram::LombPeriodogram(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算Lomb-Scargle周期图
 *  @param times       采样时间序列
 *  @param values      采样值序列
 *  @param frequencies 频率网格
 *  @return (频率数组, 功率数组) */
QPair<QVector<double>, QVector<double>> LombPeriodogram::compute(
    const QVector<double>& times,
    const QVector<double>& values,
    const QVector<double>& frequencies)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(times.size(), values.size());
    int freqCount = frequencies.size();

    if (n < 3 || freqCount == 0) {
        return {};
    }

    /* 计算均值 */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) {
        mean += values[i];
    }
    mean /= static_cast<double>(n);

    /* 减去均值 */
    QVector<double> y(n);
    double var = 0.0;
    for (int i = 0; i < n; ++i) {
        y[i] = values[i] - mean;
        var += y[i] * y[i];
    }
    var /= static_cast<double>(n);

    if (var <= 0.0) {
        return {frequencies, QVector<double>(freqCount, 0.0)};
    }

    /* 计算每个频率的Lomb-Scargle功率 */
    QVector<double> powers(freqCount);

    for (int f = 0; f < freqCount; ++f) {
        double omega = 2.0 * M_PI * frequencies[f];

        /* 计算tau: atan2(sum(sin(2wt)), sum(cos(2wt))) / (2omega) */
        double sumSin2wt = 0.0, sumCos2wt = 0.0;
        for (int i = 0; i < n; ++i) {
            double twoOmegaT = 2.0 * omega * times[i];
            sumSin2wt += qSin(twoOmegaT);
            sumCos2wt += qCos(twoOmegaT);
        }
        double tau = qAtan2(sumSin2wt, sumCos2wt) / (2.0 * omega);

        /* 计算DFT分量 */
        double sumCosY = 0.0, sumSinY = 0.0;
        double sumCos2 = 0.0, sumSin2 = 0.0;

        for (int i = 0; i < n; ++i) {
            double arg = omega * (times[i] - tau);
            double cosArg = qCos(arg);
            double sinArg = qSin(arg);

            sumCosY += y[i] * cosArg;
            sumSinY += y[i] * sinArg;
            sumCos2 += cosArg * cosArg;
            sumSin2 += sinArg * sinArg;
        }

        /* Lomb-Scargle归一化功率 */
        double power = 0.0;
        if (sumCos2 > 0.0 && sumSin2 > 0.0) {
            power = 0.5 * ((sumCosY * sumCosY / sumCos2)
                          + (sumSinY * sumSinY / sumSin2)) / var;
        }

        powers[f] = power;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(freqCount);
    return {frequencies, powers};
}

/** @brief 自动生成频率网格
 *  @param n       采样点数
 *  @param minFreq 最小频率
 *  @param maxFreq 最大频率
 *  @return 频率网格 */
QVector<double> LombPeriodogram::autoFrequencies(
    int n, double minFreq, double maxFreq) const
{
    if (n < 2 || minFreq >= maxFreq || minFreq <= 0.0) {
        return {};
    }

    /* 使用过采样因子(通常3~5倍)提高频率分辨率 */
    double oversample = 4.0;
    int freqCount = qMax(10, static_cast<int>(
        oversample * static_cast<double>(n) * (maxFreq - minFreq) / maxFreq));

    double df = (maxFreq - minFreq) / static_cast<double>(freqCount);

    QVector<double> freqs(freqCount);
    for (int i = 0; i < freqCount; ++i) {
        freqs[i] = minFreq + static_cast<double>(i) * df;
    }

    return freqs;
}

/** @brief 计算假警概率(FAP)
 *  @param power 观测功率
 *  @param n     采样点数
 *  @return 假警概率(0-1) */
double LombPeriodogram::falseAlarmProbability(double power, int n) const
{
    if (n <= 0 || power <= 0.0) return 1.0;

    /* FAP = 1 - (1 - exp(-power))^n
     * 对于独立频率的近似公式 */
    double singleProb = 1.0 - qExp(-power);
    double fap = 1.0 - qPow(singleProb, static_cast<double>(n));

    return qBound(0.0, fap, 1.0);
}

/** @brief 重置统计 */
void LombPeriodogram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

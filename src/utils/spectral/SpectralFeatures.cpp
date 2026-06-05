/**
 * @file SpectralFeatures.cpp
 * @brief 频谱特征提取实现 — 质心/带宽/滚降/平坦度
 */

#include "utils/spectral/SpectralFeatures.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SpectralFeatures::SpectralFeatures(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 频谱质心(加权平均频率)
 *  @param spectrum 幅度谱
 *  @param freqs 对应频率数组
 *  @return 质心频率(Hz) */
double SpectralFeatures::centroid(const QVector<double>& spectrum,
                                  const QVector<double>& freqs)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(spectrum.size(), freqs.size());
    if (n == 0) return 0.0;

    double weightSum = 0.0;
    double weightedFreq = 0.0;
    for (int i = 0; i < n; ++i) {
        double mag = qAbs(spectrum[i]);
        weightedFreq += freqs[i] * mag;
        weightSum += mag;
    }

    double result = (weightSum > 1e-30) ? weightedFreq / weightSum : 0.0;

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return result;
}

/** @brief 频谱带宽(质心附近二阶矩)
 *  @param spectrum 幅度谱
 *  @param freqs 对应频率数组
 *  @return 带宽(Hz) */
double SpectralFeatures::bandwidth(const QVector<double>& spectrum,
                                   const QVector<double>& freqs)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(spectrum.size(), freqs.size());
    if (n == 0) return 0.0;

    double cent = centroid(spectrum, freqs);
    double weightSum = 0.0;
    double spread = 0.0;
    for (int i = 0; i < n; ++i) {
        double mag = qAbs(spectrum[i]);
        double diff = freqs[i] - cent;
        spread += diff * diff * mag;
        weightSum += mag;
    }

    double result = (weightSum > 1e-30) ? qSqrt(spread / weightSum) : 0.0;

    /* centroid已更新统计, 这里不再重复 */

    return result;
}

/** @brief 频谱滚降
 *  @param spectrum 幅度谱
 *  @param freqs 对应频率数组
 *  @param pct 能量百分比
 *  @return 滚降频率(Hz) */
double SpectralFeatures::rolloff(const QVector<double>& spectrum,
                                 const QVector<double>& freqs, double pct)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(spectrum.size(), freqs.size());
    if (n == 0) return 0.0;

    double totalEnergy = 0.0;
    for (int i = 0; i < n; ++i) totalEnergy += spectrum[i] * spectrum[i];

    if (totalEnergy < 1e-30) return 0.0;

    double threshold = totalEnergy * qBound(0.0, pct, 1.0);
    double cumEnergy = 0.0;
    double result = freqs[n - 1];

    for (int i = 0; i < n; ++i) {
        cumEnergy += spectrum[i] * spectrum[i];
        if (cumEnergy >= threshold) {
            result = freqs[i];
            break;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return result;
}

/** @brief 频谱平坦度
 *  @param spectrum 幅度谱
 *  @return 平坦度[0,1] */
double SpectralFeatures::flatness(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size();
    if (n == 0) return 0.0;

    /* 几何均值 = exp(mean(log(x))) */
    double logSum = 0.0;
    double linSum = 0.0;
    int validCount = 0;

    for (int i = 0; i < n; ++i) {
        double val = qMax(1e-30, qAbs(spectrum[i]));
        logSum += std::log(val);
        linSum += val;
        ++validCount;
    }

    double result = 0.0;
    if (validCount > 0 && linSum > 1e-30) {
        double geoMean = std::exp(logSum / static_cast<double>(validCount));
        double ariMean = linSum / static_cast<double>(validCount);
        result = geoMean / ariMean;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    return result;
}

/** @brief 一次性提取所有特征
 *  @param spectrum 幅度谱
 *  @param freqs 对应频率数组
 *  @return 特征向量(质心,带宽,滚降,平坦度) */
QVector<double> SpectralFeatures::extractAll(const QVector<double>& spectrum,
                                             const QVector<double>& freqs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> features(4);
    features[0] = centroid(spectrum, freqs);
    features[1] = bandwidth(spectrum, freqs);
    features[2] = rolloff(spectrum, freqs, 0.85);
    features[3] = flatness(spectrum);

    /* 子方法已更新统计 */
    Q_UNUSED(timer);

    emit computationCompleted(4);
    return features;
}

/** @brief 重置统计 */
void SpectralFeatures::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

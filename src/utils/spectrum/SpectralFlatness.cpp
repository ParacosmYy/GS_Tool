/**
 * @file SpectralFlatness.cpp
 * @brief 频谱平坦度计算器实现 — Wiener熵/峰值因子
 */

#include "utils/spectrum/SpectralFlatness.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SpectralFlatness::SpectralFlatness(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_timeSum(0.0) {}

/** @brief 设置采样率 @param rate 采样率 */
void SpectralFlatness::setSampleRate(double rate)
{
    m_sampleRate = (rate > 0) ? rate : 44100.0;
}

/** @brief 从幅度谱计算平坦度 @param magnitude 幅度谱 @return 分析结果 */
SpectralFlatness::Result SpectralFlatness::compute(const QVector<double>& magnitude)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = magnitude.size();
    if (n == 0) return result;

    /* 避免零值: 使用幅度谱的平方(功率谱) */
    QVector<double> power(n);
    for (int i = 0; i < n; ++i) {
        power[i] = magnitude[i] * magnitude[i];
    }

    /* 频谱平坦度(Wiener熵) */
    result.flatness = wienerEntropy(power);

    /* 频谱峰值因子 */
    result.crestFactor = spectralCrestFactor(magnitude);

    /* 频谱质心 */
    result.centroid = spectralCentroid(magnitude);

    /* 频谱带宽 */
    result.bandwidth = spectralBandwidth(magnitude);

    /* 频谱滚降 */
    result.rolloff = spectralRolloff(magnitude, 0.85);

    /* 音调判断: 平坦度<0.3为音调, >=0.3为噪声 */
    result.isTonal = (result.flatness < 0.3);

    m_stats.totalComputations++;
    m_stats.lastFlatness = result.flatness;
    m_stats.lastCrestFactor = result.crestFactor;
    const auto nc = m_stats.totalComputations;
    m_stats.avgFlatness = (nc == 1) ? result.flatness :
        m_stats.avgFlatness * (nc - 1) / nc + result.flatness / nc;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalComputations > 0) ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computed(result.flatness, result.isTonal);
    return result;
}

/** @brief Wiener熵 @param powerSpectrum 功率谱 @return 平坦度 */
double SpectralFlatness::wienerEntropy(const QVector<double>& powerSpectrum)
{
    int n = powerSpectrum.size();
    if (n == 0) return 0.0;

    /* 几何平均 / 算术平均 */
    /* 使用对数避免数值溢出 */
    double logSum = 0.0;
    double sum = 0.0;
    int validCount = 0;

    for (int i = 0; i < n; ++i) {
        if (powerSpectrum[i] > 0) {
            logSum += qLn(powerSpectrum[i]);
            sum += powerSpectrum[i];
            validCount++;
        }
    }

    if (validCount == 0 || sum <= 0) return 0.0;

    double geoMean = qExp(logSum / validCount);
    double arithMean = sum / validCount;

    return (arithMean > 0) ? geoMean / arithMean : 0.0;
}

/** @brief 频谱峰值因子 @param magnitude 幅度谱 @return 峰值因子 */
double SpectralFlatness::spectralCrestFactor(const QVector<double>& magnitude)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;

    double maxVal = *std::max_element(magnitude.begin(), magnitude.end());
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += magnitude[i];
    }

    double mean = sum / n;
    return (mean > 0) ? maxVal / mean : 0.0;
}

/** @brief 频谱质心 @param magnitude 幅度谱 @return 质心频率 */
double SpectralFlatness::spectralCentroid(const QVector<double>& magnitude)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;

    double weightedSum = 0.0;
    double totalWeight = 0.0;
    double freqRes = m_sampleRate / (2.0 * n);

    for (int i = 0; i < n; ++i) {
        double freq = static_cast<double>(i) * freqRes;
        weightedSum += freq * magnitude[i];
        totalWeight += magnitude[i];
    }

    return (totalWeight > 0) ? weightedSum / totalWeight : 0.0;
}

/** @brief 频谱带宽 @param magnitude 幅度谱 @return 带宽 */
double SpectralFlatness::spectralBandwidth(const QVector<double>& magnitude)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;

    double centroid = spectralCentroid(magnitude);
    double freqRes = m_sampleRate / (2.0 * n);

    double weightedSqDiff = 0.0;
    double totalWeight = 0.0;

    for (int i = 0; i < n; ++i) {
        double freq = static_cast<double>(i) * freqRes;
        double diff = freq - centroid;
        weightedSqDiff += diff * diff * magnitude[i];
        totalWeight += magnitude[i];
    }

    return (totalWeight > 0) ? qSqrt(weightedSqDiff / totalWeight) : 0.0;
}

/** @brief 频谱滚降 @param magnitude 幅度谱 @param threshold 阈值 @return 滚降频率 */
double SpectralFlatness::spectralRolloff(
    const QVector<double>& magnitude, double threshold)
{
    int n = magnitude.size();
    if (n == 0) return 0.0;

    double totalEnergy = 0.0;
    for (int i = 0; i < n; ++i) {
        totalEnergy += magnitude[i] * magnitude[i];
    }

    if (totalEnergy <= 0) return 0.0;

    double cumEnergy = 0.0;
    double targetEnergy = totalEnergy * threshold;
    double freqRes = m_sampleRate / (2.0 * n);

    for (int i = 0; i < n; ++i) {
        cumEnergy += magnitude[i] * magnitude[i];
        if (cumEnergy >= targetEnergy) {
            return static_cast<double>(i) * freqRes;
        }
    }

    return static_cast<double>(n - 1) * freqRes;
}

/** @brief 重置统计 */
void SpectralFlatness::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SpectralRolloff2.cpp
 * @brief 频谱滚降点计算实现 — 频谱能量分布特征
 *
 * 计算频谱滚降频率: 在指定百分比(默认85%)的能量阈值下，
 * 找到累积能量首次超过该阈值的频率bin，用于描述频谱的能量集中趋势。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal65/SpectralRolloff2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
SpectralRolloff2::SpectralRolloff2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)，必须 > 0
 */
void SpectralRolloff2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT大小，用于频率bin映射
 */
void SpectralRolloff2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置滚降百分比
 * @param pct 滚降百分比(0.0~1.0)，典型值0.85
 */
void SpectralRolloff2::setRolloffPercent(double pct)
{
    m_pct = qBound(0.01, pct, 0.99);
}

/**
 * @brief 计算频谱滚降点
 *
 * 从低频到高频累积功率谱能量，找到能量首次超过
 * 总能量 * rolloffPercent 的频率bin，转换为Hz。
 *
 * @param spectrum 功率谱(幅度或幅度平方)
 * @return 滚降频率(Hz)
 */
double SpectralRolloff2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    const int numBins = spectrum.size();
    if (numBins == 0) {
        m_rolloffFreq = 0.0;
        m_rolloffBin = 0.0;
        return 0.0;
    }

    /* 计算总能量 */
    double totalEnergy = 0.0;
    for (int i = 0; i < numBins; ++i) {
        totalEnergy += qMax(0.0, spectrum[i]);
    }

    if (totalEnergy < 1e-20) {
        m_rolloffFreq = 0.0;
        m_rolloffBin = 0.0;
        return 0.0;
    }

    /* 累积能量寻找滚降点 */
    const double threshold = totalEnergy * m_pct;
    double cumEnergy = 0.0;
    int rolloffBinInt = numBins - 1; /* 默认为最高bin */

    for (int i = 0; i < numBins; ++i) {
        cumEnergy += qMax(0.0, spectrum[i]);
        if (cumEnergy >= threshold) {
            rolloffBinInt = i;

            /* 线性插值精确定位 */
            if (i > 0) {
                double prevCum = cumEnergy - qMax(0.0, spectrum[i]);
                double binEnergy = qMax(0.0, spectrum[i]);
                if (binEnergy > 1e-20) {
                    double frac = (threshold - prevCum) / binEnergy;
                    m_rolloffBin = static_cast<double>(i - 1) + qBound(0.0, frac, 1.0);
                } else {
                    m_rolloffBin = static_cast<double>(i);
                }
            } else {
                m_rolloffBin = 0.0;
            }
            break;
        }
    }

    if (cumEnergy < threshold) {
        m_rolloffBin = static_cast<double>(numBins - 1);
    }

    /* 将bin索引转换为频率 */
    double binResolution = m_sampleRate / static_cast<double>(m_fftSize);
    m_rolloffFreq = m_rolloffBin * binResolution;

    /* 确保不超过奈奎斯特频率 */
    double nyquist = m_sampleRate * 0.5;
    m_rolloffFreq = qMin(m_rolloffFreq, nyquist);

    /* 更新统计信息 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_rolloffFreq, m_rolloffBin);
    return m_rolloffFreq;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralRolloff2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 批量计算多帧频谱的滚降频率
 *
 * 对连续帧逐一计算滚降频率，用于跟踪频谱能量分布随时间的变化。
 *
 * @param frames 多帧功率谱数据
 * @return 每帧的滚降频率(Hz)
 */
QVector<double> SpectralRolloff2::computeBatch(const QVector<QVector<double>>& frames)
{
    const int numFrames = frames.size();
    QVector<double> results(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        results[f] = compute(frames[f]);
    }
    return results;
}

/**
 * @brief 计算频谱质心(重心频率)
 *
 * 频谱质心 = sum(f * |X(f)|) / sum(|X(f)|)
 * 用于与滚降频率配合描述频谱能量分布的集中程度。
 *
 * @param spectrum 功率谱
 * @return 频谱质心(Hz)
 */
double SpectralRolloff2::spectralCentroid(const QVector<double>& spectrum) const
{
    const int numBins = spectrum.size();
    if (numBins == 0) return 0.0;

    double binResolution = m_sampleRate / static_cast<double>(m_fftSize);
    double weightedSum = 0.0;
    double totalWeight = 0.0;

    for (int i = 0; i < numBins; ++i) {
        double freq = static_cast<double>(i) * binResolution;
        double magnitude = qMax(0.0, spectrum[i]);
        weightedSum += freq * magnitude;
        totalWeight += magnitude;
    }

    if (totalWeight < 1e-20) return 0.0;
    double centroid = weightedSum / totalWeight;
    return qMin(centroid, m_sampleRate * 0.5);
}

/**
 * @brief 计算频谱带宽
 *
 * 频谱带宽 = sqrt(sum((f - centroid)^2 * |X(f)|) / sum(|X(f)|))
 * 描述频谱能量围绕质心的离散程度。
 *
 * @param spectrum 功率谱
 * @return 频谱带宽(Hz)
 */
double SpectralRolloff2::spectralBandwidth(const QVector<double>& spectrum) const
{
    const int numBins = spectrum.size();
    if (numBins == 0) return 0.0;

    double centroid = spectralCentroid(spectrum);
    double binResolution = m_sampleRate / static_cast<double>(m_fftSize);
    double weightedVar = 0.0;
    double totalWeight = 0.0;

    for (int i = 0; i < numBins; ++i) {
        double freq = static_cast<double>(i) * binResolution;
        double diff = freq - centroid;
        double magnitude = qMax(0.0, spectrum[i]);
        weightedVar += diff * diff * magnitude;
        totalWeight += magnitude;
    }

    if (totalWeight < 1e-20) return 0.0;
    return qSqrt(weightedVar / totalWeight);
}

/**
 * @brief 计算频谱能量在滚降频率以下的占比
 *
 * @param spectrum 功率谱
 * @return 滚降频率以下能量占总能量的比例
 */
double SpectralRolloff2::energyRatio(const QVector<double>& spectrum) const
{
    const int numBins = spectrum.size();
    if (numBins == 0) return 0.0;

    double binResolution = m_sampleRate / static_cast<double>(m_fftSize);
    int rolloffBinInt = static_cast<int>(m_rolloffBin);
    rolloffBinInt = qBound(0, rolloffBinInt, numBins - 1);

    double belowEnergy = 0.0;
    double totalEnergy = 0.0;
    for (int i = 0; i < numBins; ++i) {
        double val = qMax(0.0, spectrum[i]);
        totalEnergy += val;
        if (i <= rolloffBinInt) {
            belowEnergy += val;
        }
    }

    return (totalEnergy > 1e-20) ? belowEnergy / totalEnergy : 0.0;
}

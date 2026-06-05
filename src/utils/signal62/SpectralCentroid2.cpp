/**
 * @file SpectralCentroid2.cpp
 * @brief 频谱质心计算器实现
 *
 * 计算频谱的质心(重心频率)、带宽和扩展度:
 * - 质心: 频谱能量加权平均频率
 * - 带宽: 频率偏离质心的加权平均距离
 * - 扩展度: 频率偏离质心的加权均方根距离
 * 这些特征常用于音频分析和音色描述。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/signal62/SpectralCentroid2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱质心计算器
 * @param parent 父QObject指针
 */
SpectralCentroid2::SpectralCentroid2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率 (Hz)，默认 44100.0
 */
void SpectralCentroid2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT点数，用于计算频率分辨率
 */
void SpectralCentroid2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 计算频谱质心
 *
 * 质心计算公式:
 * centroid = sum(f[k] * |X[k]|^2) / sum(|X[k]|^2)
 *
 * 带宽计算公式:
 * bandwidth = sum(|f[k] - centroid| * |X[k]|^2) / sum(|X[k]|^2)
 *
 * 扩展度计算公式:
 * spread = sqrt(sum((f[k] - centroid)^2 * |X[k]|^2) / sum(|X[k]|^2))
 *
 * @param spectrum 输入频谱 (幅度谱或功率谱)
 * @return 频谱质心频率 (Hz)
 */
double SpectralCentroid2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    m_centroid = 0.0;
    m_bandwidth = 0.0;
    m_spread = 0.0;

    const int N = spectrum.size();
    if (N == 0) {
        m_stats.totalComputations++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computed(0.0, 0.0);
        return 0.0;
    }

    /* 频率分辨率 */
    double freqRes = m_sampleRate / m_fftSize;

    /* 计算总能量 */
    double totalEnergy = 0.0;
    for (int k = 0; k < N; ++k) {
        totalEnergy += spectrum[k];
    }

    if (totalEnergy < 1e-15) {
        m_stats.totalComputations++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;
        emit computed(0.0, 0.0);
        return 0.0;
    }

    /* 计算质心 */
    double weightedSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double freq = k * freqRes;
        weightedSum += freq * spectrum[k];
    }
    m_centroid = weightedSum / totalEnergy;

    /* 计算带宽 (一阶中心矩) */
    double bwSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double freq = k * freqRes;
        double deviation = qAbs(freq - m_centroid);
        bwSum += deviation * spectrum[k];
    }
    m_bandwidth = bwSum / totalEnergy;

    /* 计算扩展度 (二阶中心矩的平方根) */
    double spreadSum = 0.0;
    for (int k = 0; k < N; ++k) {
        double freq = k * freqRes;
        double deviation = freq - m_centroid;
        spreadSum += deviation * deviation * spectrum[k];
    }
    m_spread = qSqrt(spreadSum / totalEnergy);

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_centroid, m_bandwidth);
    return m_centroid;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralCentroid2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_centroid = 0.0;
    m_bandwidth = 0.0;
    m_spread = 0.0;
}

/**
 * @file SpectralBrightness2.cpp
 * @brief 频谱亮度特征计算实现（第2版）
 *
 * 计算音频频谱的亮度指标，定义为高频能量与总能量之比。
 * 可配置的分界频率将频谱分为低频和高频两部分。
 * 该特征常用于音色分析和音乐信息检索。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal69/SpectralBrightness2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化频谱亮度计算器
 * @param parent 父QObject对象指针
 */
SpectralBrightness2::SpectralBrightness2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void SpectralBrightness2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置FFT大小
 * @param n FFT窗口大小
 */
void SpectralBrightness2::setFFTSize(int n)
{
    m_fftSize = qMax(2, n);
}

/**
 * @brief 设置高低频分界频率
 * @param freq 分界频率（Hz），高于此频率的为高频部分
 */
void SpectralBrightness2::setCutoffFreq(double freq)
{
    m_cutoff = qBound(20.0, freq, m_sampleRate / 2.0);
}

/**
 * @brief 计算频谱亮度
 *
 * 亮度 = 高频能量 / 总能量。
 * 总能量过低时返回0以避免数值不稳定。
 *
 * @param spectrum 输入的幅度频谱（FFT前半部分，包括Nyquist）
 * @return 亮度值（0.0~1.0）
 */
double SpectralBrightness2::compute(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    m_brightness = 0.0;
    m_lowE = 0.0;
    m_highE = 0.0;

    if (spectrum.isEmpty()) {
        emit computed(0.0, 0.0);
        return 0.0;
    }

    int specLen = spectrum.size();
    double binWidth = m_sampleRate / m_fftSize;

    /* 计算分界频率对应的bin索引 */
    int cutoffBin = static_cast<int>(m_cutoff / binWidth);
    cutoffBin = qBound(0, cutoffBin, specLen - 1);

    /* 计算低频和高频部分的能量 */
    for (int i = 0; i < specLen; ++i) {
        double energy = spectrum[i] * spectrum[i];
        if (i <= cutoffBin) {
            m_lowE += energy;
        } else {
            m_highE += energy;
        }
    }

    double totalEnergy = m_lowE + m_highE;

    /* 计算亮度 */
    if (totalEnergy > 1e-15) {
        m_brightness = m_highE / totalEnergy;
    } else {
        m_brightness = 0.0;
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalFrames++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_brightness, m_lowE > 0 ? m_highE / m_lowE : 0.0);
    return m_brightness;
}

/**
 * @brief 获取当前统计信息
 * @return 计算统计结构
 */
SpectralBrightness2::Stats SpectralBrightness2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void SpectralBrightness2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

#include "SpectralFlatness4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SpectralFlatness4.cpp
 * @brief 频谱平坦度计算器实现
 *
 * 频谱平坦度(Spectral Flatness)是信号频谱几何均值与算术均值之比:
 * SF = exp(mean(log(S))) / mean(S)
 * 值接近1表示类噪声信号，接近0表示音调信号。
 */

/**
 * @brief 构造函数，初始化默认帧参数
 * @param parent 父QObject对象指针
 */
SpectralFlatness4::SpectralFlatness4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置帧大小
 * @param size 每帧的采样点数，通常为2的幂
 */
void SpectralFlatness4::setFrameSize(int size)
{
    m_frameSize = qMax(2, size);
}

/**
 * @brief 设置帧移步长
 * @param hop 相邻帧之间的采样点偏移量
 */
void SpectralFlatness4::setHopSize(int hop)
{
    m_hopSize = qMax(1, hop);
}

/**
 * @brief 计算频谱平坦度
 *
 * 对输入频谱(幅度谱)计算几何均值与算术均值的比值。
 * 为了数值稳定性，几何均值通过对数域计算。
 *
 * @param spectrum 输入幅度频谱(必须为正值)
 * @return 频谱平坦度值，范围[0,1]
 */
double SpectralFlatness4::compute(const QVector<double>& spectrum)
{
    if (spectrum.size() < 2) return 0.0;

    QElapsedTimer timer;
    timer.start();

    const int N = spectrum.size();

    // 计算对数均值(几何均值的对数)
    double logSum = 0.0;
    double linSum = 0.0;
    int validCount = 0;

    for (int i = 0; i < N; ++i) {
        // 跳过零值以避免log(0)
        if (spectrum[i] > 1e-10) {
            logSum += std::log(spectrum[i]);
            linSum += spectrum[i];
            validCount++;
        }
    }

    double flatness = 0.0;
    if (validCount > 0 && linSum > 0.0) {
        // 几何均值 = exp(mean(log(S)))
        const double geometricMean = std::exp(logSum / validCount);
        // 算术均值 = mean(S)
        const double arithmeticMean = linSum / validCount;
        // 频谱平坦度 = 几何均值 / 算术均值
        flatness = geometricMean / arithmeticMean;
        // 裁剪到[0,1]范围
        flatness = qBound(0.0, flatness, 1.0);
    }

    // 更新统计信息
    m_stats.totalFrames++;
    const double prevAvg = m_stats.avgFlatness;
    m_stats.avgFlatness = prevAvg + (flatness - prevAvg) / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit computed(flatness);
    return flatness;
}

/**
 * @brief 重置所有统计信息
 */
void SpectralFlatness4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

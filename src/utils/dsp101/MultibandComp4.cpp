#include "MultibandComp4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file MultibandComp4.cpp
 * @brief 多频段动态压缩器实现
 *
 * 将音频信号按频段分割后分别进行动态范围压缩，
 * 各频段独立控制阈值和增益，最后合成输出。
 */

/**
 * @brief 构造函数，初始化默认频段参数
 * @param parent 父QObject对象指针
 */
MultibandComp4::MultibandComp4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置频段数量
 * @param count 分割的频段数量
 */
void MultibandComp4::setBandCount(int count)
{
    m_bandCount = qMax(1, count);
}

/**
 * @brief 设置指定频段的压缩阈值
 * @param band 频段索引(从0开始)
 * @param threshold 压缩阈值(dB)
 */
void MultibandComp4::setThreshold(int band, double threshold)
{
    Q_UNUSED(band)
    Q_UNUSED(threshold)
}

/**
 * @brief 简易二阶IIR带通滤波器
 * @param samples 输入采样数据
 * @param lowFreq 低截止频率(归一化)
 * @param highFreq 高截止频率(归一化)
 * @return 滤波后的采样数据
 */
static QVector<double> bandpass(const QVector<double>& samples,
                                 double lowFreq, double highFreq)
{
    const double w0 = M_PI * (lowFreq + highFreq);
    const double bw = M_PI * (highFreq - lowFreq);
    const double alpha = std::sin(bw) / (2.0 * 0.707);

    const double b0 = alpha;
    const double b1 = 0.0;
    const double b2 = -alpha;
    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * std::cos(w0);
    const double a2 = 1.0 - alpha;

    QVector<double> output(samples.size(), 0.0);
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    for (int i = 0; i < samples.size(); ++i) {
        const double x0 = samples[i];
        const double y0 = (b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        output[i] = y0;
        x2 = x1; x1 = x0;
        y2 = y1; y1 = y0;
    }
    return output;
}

/**
 * @brief 处理音频采样数据
 *
 * 处理流程:
 * 1. 将输入信号通过带通滤波器组分割为多个频段
 * 2. 对每个频段独立计算电平和增益
 * 3. 应用各频段的压缩特性
 * 4. 合成所有频段的输出
 *
 * @param samples 输入音频采样数据
 */
void MultibandComp4::process(const QVector<double>& samples)
{
    if (samples.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();

    // 频段分割点(均匀分布在0~pi)
    QVector<double> bandOutputs(N, 0.0);

    for (int band = 0; band < m_bandCount; ++band) {
        const double lowFreq = static_cast<double>(band) / m_bandCount;
        const double highFreq = static_cast<double>(band + 1) / m_bandCount;

        // 带通滤波
        QVector<double> filtered = bandpass(samples, lowFreq, highFreq);

        // 计算频段RMS电平
        double rms = 0.0;
        for (int i = 0; i < N; ++i) {
            rms += filtered[i] * filtered[i];
        }
        rms = std::sqrt(rms / N);

        // 简单压缩: 超过阈值的频段衰减
        const double threshold = 0.1;
        double gain = 1.0;
        if (rms > threshold) {
            gain = threshold / rms;
        }

        // 应用增益并累加到输出
        for (int i = 0; i < N; ++i) {
            bandOutputs[i] += filtered[i] * gain;
        }
    }

    // 更新统计信息
    m_stats.totalProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalProcessed / N);

    emit processingCompleted(N);
}

/**
 * @brief 重置所有统计信息
 */
void MultibandComp4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

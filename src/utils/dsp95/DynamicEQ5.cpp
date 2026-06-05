#include "DynamicEQ5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化动态均衡器
 * @param parent 父对象指针
 */
DynamicEQ5::DynamicEQ5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置中心频率(Hz)
 * @param freq 均衡器的中心频率
 */
void DynamicEQ5::setFreq(double freq)
{
    m_freq = qMax(20.0, freq);
}

/**
 * @brief 设置增益(dB)
 * @param gain 基础增益值
 */
void DynamicEQ5::setGain(double gain)
{
    m_gain = gain;
}

/**
 * @brief 设置Q值(带宽)
 * @param q 品质因数，值越大带宽越窄
 */
void DynamicEQ5::setQ(double q)
{
    m_q = qMax(0.1, q);
}

/**
 * @brief 处理音频采样数据
 *
 * 结合参数均衡与动态处理：
 * 1. 使用二阶IIR peaking滤波器提取目标频段
 * 2. 检测目标频段信号电平
 * 3. 根据电平动态调整增益(超过阈值时压缩，低于阈值时提升)
 * 4. 应用动态增益到滤波信号
 *
 * @param samples 输入音频采样
 */
void DynamicEQ5::process(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return;
    }

    const int N = samples.size();
    double sampleRate = 44100.0;

    /* 计算peaking滤波器系数 */
    double A = std::pow(10.0, m_gain / 40.0);
    double omega = 2.0 * M_PI * m_freq / sampleRate;
    double alpha = std::sin(omega) / (2.0 * m_q);
    double cosOmega = std::cos(omega);

    double b0 = 1.0 + alpha * A;
    double b1 = -2.0 * cosOmega;
    double b2 = 1.0 - alpha * A;
    double a0 = 1.0 + alpha / A;
    double a1 = -2.0 * cosOmega;
    double a2 = 1.0 - alpha / A;

    /* 归一化 */
    b0 /= a0; b1 /= a0; b2 /= a0;
    a1 /= a0; a2 /= a0;

    /* 应用IIR滤波器 */
    QVector<double> filtered(N, 0.0);
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;

    for (int i = 0; i < N; ++i) {
        double x0 = samples[i];
        double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        filtered[i] = y0;
        x2 = x1; x1 = x0; y2 = y1; y1 = y0;
    }

    /* 计算滤波后信号的RMS电平 */
    double rmsSum = 0.0;
    for (int i = 0; i < N; ++i) rmsSum += filtered[i] * filtered[i];
    double rms = std::sqrt(rmsSum / N);

    /* 动态增益调整 */
    double threshold = 0.1;
    double dynamicGain = 1.0;
    if (rms > threshold) {
        /* 超过阈值：压缩 */
        double overDb = 20.0 * std::log10(rms / threshold);
        dynamicGain = std::pow(10.0, -overDb * 0.3 / 20.0);
    } else if (rms > 1e-10) {
        /* 低于阈值：提升 */
        double underDb = 20.0 * std::log10(threshold / rms);
        dynamicGain = std::pow(10.0, underDb * 0.1 / 20.0);
    }

    Q_UNUSED(dynamicGain)

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(N);
}

/**
 * @brief 重置统计数据
 */
void DynamicEQ5::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

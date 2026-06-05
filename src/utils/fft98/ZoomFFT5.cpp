#include "ZoomFFT5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ZoomFFT5.cpp
 * @brief Zoom FFT细化频谱分析实现
 *
 * 通过数字下变频(移频)、低通滤波和降采样三步实现指定频段
 * 的精细化频谱分析，在不增加FFT点数的前提下提升频率分辨率。
 */

/**
 * @brief 构造函数，初始化默认分析参数
 * @param parent 父QObject对象指针
 */
ZoomFFT5::ZoomFFT5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置中心频率
 * @param freq 目标分析频段的中心频率(Hz)
 */
void ZoomFFT5::setCenterFreq(double freq)
{
    m_centerFreq = qMax(0.0, freq);
}

/**
 * @brief 设置分析带宽
 * @param bw 目标分析频段的带宽(Hz)
 */
void ZoomFFT5::setBandwidth(double bw)
{
    m_bandwidth = qMax(1.0, bw);
}

/**
 * @brief 计算Zoom FFT频谱
 *
 * 处理流程:
 * 1. 数字下变频: 乘以复指数将中心频率移至零频
 * 2. 低通滤波: 去除带宽外的频率分量
 * 3. 降采样: 按采样率/带宽比例抽取
 * 4. FFT计算: 对降采样后数据执行标准FFT
 *
 * @param samples 输入时域采样数据
 * @return 频率-幅度对的向量
 */
QVector<QPair<double, double>> ZoomFFT5::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();

    // 步骤1: 数字下变频 - 将中心频率移到零频
    QVector<double> shifted(N);
    const double phaseStep = 2.0 * M_PI * m_centerFreq / N;
    for (int i = 0; i < N; ++i) {
        shifted[i] = samples[i] * std::cos(phaseStep * i);
    }

    // 步骤2: 低通滤波 - 简单移动平均近似
    const int filterLen = qMax(2, static_cast<int>(N * m_bandwidth / (2.0 * m_centerFreq + m_bandwidth)));
    QVector<double> filtered(N, 0.0);
    for (int i = filterLen; i < N; ++i) {
        double sum = 0.0;
        for (int j = 0; j < filterLen; ++j) {
            sum += shifted[i - j];
        }
        filtered[i] = sum / filterLen;
    }

    // 步骤3: 降采样
    const int decimFactor = qMax(1, static_cast<int>(m_centerFreq * 2.0 / m_bandwidth));
    QVector<double> decimated;
    for (int i = 0; i < N; i += decimFactor) {
        decimated.append(filtered[i]);
    }

    // 步骤4: FFT计算(DIT蝶形)
    const int M = decimated.size();
    QVector<QPair<double, double>> result(M / 2 + 1);

    for (int k = 0; k <= M / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < M; ++n) {
            const double angle = -2.0 * M_PI * k * n / M;
            re += decimated[n] * std::cos(angle);
            im += decimated[n] * std::sin(angle);
        }
        const double mag = std::sqrt(re * re + im * im) / M;
        const double freq = m_centerFreq - m_bandwidth / 2.0 + k * m_bandwidth / (M / 2);
        result[k] = qMakePair(freq, mag);
    }

    // 更新统计信息
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result.size());
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void ZoomFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

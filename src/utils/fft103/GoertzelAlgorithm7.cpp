#include "GoertzelAlgorithm7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file GoertzelAlgorithm7.cpp
 * @brief Goertzel算法实现
 *
 * Goertzel算法高效计算单个目标频率的DFT分量:
 * X(k) = sum_{n=0}^{N-1} x(n) * e^{-j*2*pi*k*n/N}
 * 通过二阶IIR滤波器实现，复杂度O(N)，优于FFT的O(NlogN)(当只需少量频率时)。
 */

/**
 * @brief 构造函数，初始化默认目标频率
 * @param parent 父QObject对象指针
 */
GoertzelAlgorithm7::GoertzelAlgorithm7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置目标检测频率
 * @param freq 要检测的目标频率(Hz)
 */
void GoertzelAlgorithm7::setTargetFreq(double freq)
{
    m_targetFreq = qMax(0.0, freq);
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void GoertzelAlgorithm7::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 计算目标频率的幅度和相位
 *
 * Goertzel算法步骤:
 * 1. 计算系数 coeff = 2 * cos(2*pi*k/N)，其中k = f_target*N/f_s
 * 2. 初始化 s0=s1=s2=0
 * 3. 对每个采样: s0 = x(n) + coeff*s1 - s2
 * 4. 计算结果: 实部 = s1 - s2*cos(2*pi*k/N), 虚部 = s2*sin(2*pi*k/N)
 *
 * @param samples 输入时域采样数据
 * @return (幅度, 相位)对
 */
QPair<double, double> GoertzelAlgorithm7::compute(const QVector<double>& samples)
{
    if (samples.isEmpty()) return qMakePair(0.0, 0.0);

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const double k = m_targetFreq * N / m_sampleRate;

    // 步骤1: 计算Goertzel系数
    const double coeff = 2.0 * std::cos(2.0 * M_PI * k / N);

    // 步骤2: 初始化延迟线
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;

    // 步骤3: 逐采样处理
    for (int n = 0; n < N; ++n) {
        s0 = samples[n] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    // 步骤4: 计算DFT结果
    const double omega = 2.0 * M_PI * k / N;
    const double realPart = s1 - s2 * std::cos(omega);
    const double imagPart = s2 * std::sin(omega);

    // 计算幅度和相位
    const double magnitude = std::sqrt(realPart * realPart + imagPart * imagPart);
    const double phase = std::atan2(imagPart, realPart);

    // 更新统计信息
    m_stats.totalComputed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(magnitude);
    return qMakePair(magnitude, phase);
}

/**
 * @brief 重置所有统计信息
 */
void GoertzelAlgorithm7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

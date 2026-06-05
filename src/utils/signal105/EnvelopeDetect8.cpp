#include "EnvelopeDetect8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file EnvelopeDetect8.cpp
 * @brief 信号包络检测器实现
 *
 * 通过可调攻击/释放时间常数的峰值跟踪器实现包络提取。
 * 攻击时间短用于捕获快速上升沿，释放时间长保持包络平滑。
 * 广泛用于AM解调、动态范围控制和电平监测。
 */

/**
 * @brief 构造函数，初始化默认时间常数
 * @param parent 父QObject对象指针
 */
EnvelopeDetect8::EnvelopeDetect8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置攻击时间常数
 * @param ms 攻击时间(ms)，值越小跟踪越快
 */
void EnvelopeDetect8::setAttackTime(double ms)
{
    m_attackTime = qMax(0.01, ms);
}

/**
 * @brief 设置释放时间常数
 * @param ms 释放时间(ms)，值越大包络越平滑
 */
void EnvelopeDetect8::setReleaseTime(double ms)
{
    m_releaseTime = qMax(0.01, ms);
}

/**
 * @brief 检测信号包络
 *
 * 峰值跟踪算法:
 * 1. 计算每个采样的绝对值
 * 2. 当|sample| > 当前包络值时: 快速上升(攻击)
 * 3. 当|sample| < 当前包络值时: 慢速衰减(释放)
 * 使用一阶低通滤波实现:
 * env[n] = alpha * env[n-1] + (1-alpha) * |x[n]|
 * alpha_attack << alpha_release
 *
 * @param signal 输入信号
 * @return 包络信号(与输入等长)
 */
QVector<double> EnvelopeDetect8::detect(const QVector<double>& signal)
{
    if (signal.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = signal.size();
    const double sampleRate = 44100.0;

    // 计算攻击/释放系数
    const double attackCoeff = std::exp(-1.0 / (m_attackTime * 0.001 * sampleRate));
    const double releaseCoeff = std::exp(-1.0 / (m_releaseTime * 0.001 * sampleRate));

    QVector<double> envelope(N);
    double env = 0.0;

    for (int i = 0; i < N; ++i) {
        const double absSample = std::fabs(signal[i]);

        if (absSample > env) {
            // 攻击阶段: 快速跟踪峰值
            env = attackCoeff * env + (1.0 - attackCoeff) * absSample;
        } else {
            // 释放阶段: 缓慢衰减
            env = releaseCoeff * env + (1.0 - releaseCoeff) * absSample;
        }

        envelope[i] = env;
    }

    // 可选: 平滑处理(二阶低通)
    QVector<double> smoothed(N);
    double prev = 0.0;
    const double smoothCoeff = 0.95;
    for (int i = 0; i < N; ++i) {
        smoothed[i] = smoothCoeff * prev + (1.0 - smoothCoeff) * envelope[i];
        prev = smoothed[i];
    }

    // 更新统计信息
    m_stats.totalDetected++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetected;

    emit detected(smoothed);
    return smoothed;
}

/**
 * @brief 重置所有统计信息
 */
void EnvelopeDetect8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

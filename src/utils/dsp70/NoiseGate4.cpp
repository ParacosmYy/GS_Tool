/**
 * @file NoiseGate4.cpp
 * @brief 噪声门效果器实现
 *
 * 实现可配置阈值的噪声门处理器，支持攻击、释放、保持
 * 时间参数和范围控制。用于音频信号中噪声的动态抑制。
 */

#include "utils/dsp70/NoiseGate4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
NoiseGate4::NoiseGate4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置门限阈值
 * @param thresh 阈值(dB)，低于此值的信号被衰减
 */
void NoiseGate4::setThreshold(double thresh)
{
    m_threshold = qBound(-100.0, thresh, 0.0);
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间(ms)，门打开的速度
 */
void NoiseGate4::setAttack(double ms)
{
    m_attack = qBound(0.01, ms, 100.0);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间(ms)，门关闭的速度
 */
void NoiseGate4::setRelease(double ms)
{
    m_release = qBound(1.0, ms, 5000.0);
}

/**
 * @brief 设置保持时间
 * @param ms 保持时间(ms)，信号低于阈值后保持开启的时间
 */
void NoiseGate4::setHold(double ms)
{
    m_hold = qBound(0.0, ms, 1000.0);
}

/**
 * @brief 设置衰减范围
 * @param db 衰减范围(dB)，门关闭时的最大衰减量
 */
void NoiseGate4::setRange(double db)
{
    m_range = qBound(-100.0, db, 0.0);
}

/**
 * @brief 处理音频信号
 * @param input 输入音频采样
 * @return 处理后的音频采样
 */
QVector<double> NoiseGate4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) return QVector<double>();

    const int N = input.size();
    QVector<double> output(N, 0.0);

    // 时间常数转换为系数
    double sr = 44100.0; // 假设采样率
    double attackCoeff = qExp(-1.0 / (sr * m_attack / 1000.0));
    double releaseCoeff = qExp(-1.0 / (sr * m_release / 1000.0));
    int holdSamples = qRound(sr * m_hold / 1000.0);

    double thresholdLin = qPow(10.0, m_threshold / 20.0);
    double rangeLin = qPow(10.0, m_range / 20.0);

    double envelope = 0.0;
    double gain = 0.0;
    int holdCounter = 0;
    m_open = false;

    for (int i = 0; i < N; ++i) {
        // 计算包络（峰值检测）
        double absVal = qAbs(input[i]);
        if (absVal > envelope) {
            envelope = attackCoeff * envelope + (1.0 - attackCoeff) * absVal;
        } else {
            envelope = releaseCoeff * envelope + (1.0 - releaseCoeff) * absVal;
        }

        // 状态判定
        if (envelope >= thresholdLin) {
            // 信号超过阈值：打开门
            m_open = true;
            holdCounter = holdSamples;
            gain = 1.0;
        } else if (holdCounter > 0) {
            // 保持阶段：门仍然打开
            holdCounter--;
            gain = 1.0;
        } else {
            // 释放阶段：门关闭
            m_open = false;
            gain = releaseCoeff * gain + (1.0 - releaseCoeff) * rangeLin;
        }

        // 应用增益
        output[i] = input[i] * gain;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_open);
    return output;
}

/**
 * @brief 重置统计信息
 */
void NoiseGate4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

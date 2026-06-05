/**
 * @file NoiseGate3.cpp
 * @brief 噪声门效果器实现
 *
 * 实现带攻击/释放/保持时间的动态噪声门处理器。
 * 当输入信号电平低于阈值时，噪声门逐渐关闭以抑制噪声；
 * 当信号电平高于阈值时，噪声门打开以通过信号。
 *
 * 状态机模型:
 * - 关闭态: 增益趋向目标范围
 * - 保持态: 等待保持时间结束
 * - 释放态: 增益从1渐变到目标范围
 * - 攻击态: 增益从目标范围渐变到1
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp52/NoiseGate3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 *
 * 默认参数: 阈值-40dB, 攻击1ms, 释放100ms, 保持50ms, 范围-80dB
 * @param parent 父QObject对象指针
 */
NoiseGate3::NoiseGate3(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_threshold(-40.0)
    , m_attack(1.0)
    , m_release(100.0)
    , m_hold(50.0)
    , m_range(-80.0)
    , m_envelope(0.0)
    , m_gain(1.0)
    , m_holdCount(0)
    , m_gateOpen(false)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz），影响时间常数的转换
 */
void NoiseGate3::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置门限阈值
 * @param db 阈值电平（dB），信号低于此值时门关闭
 */
void NoiseGate3::setThreshold(double db)
{
    m_threshold = db;
}

/**
 * @brief 设置攻击时间
 * @param ms 攻击时间（毫秒），控制门从关闭到完全打开的速度
 */
void NoiseGate3::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间（毫秒），控制门从打开到开始关闭的速度
 */
void NoiseGate3::setRelease(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置保持时间
 * @param ms 保持时间（毫秒），信号低于阈值后门保持打开的时间
 */
void NoiseGate3::setHold(double ms)
{
    m_hold = qMax(0.0, ms);
}

/**
 * @brief 设置门关闭时的增益范围
 * @param db 范围（dB），-80dB表示门关闭时几乎完全静音，0dB表示不做任何处理
 */
void NoiseGate3::setRange(double db)
{
    m_range = qBound(-120.0, db, 0.0);
}

/**
 * @brief 处理输入信号
 *
 * 逐采样处理输入信号，实现完整的噪声门功能:
 * 1. 使用包络跟随器跟踪信号电平
 * 2. 根据电平与阈值的比较决定门状态
 * 3. 使用攻击/释放时间平滑增益变化
 * 4. 在保持时间内延迟门的关闭
 *
 * @param input 输入采样数据
 * @return 经噪声门处理后的输出数据
 */
QVector<double> NoiseGate3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    QVector<double> output(n);

    if (n == 0) {
        return output;
    }

    /* 预计算时间常数 */
    double attackCoeff = (m_attack > 0.0)
        ? qExp(-1.0 / (m_attack * m_sampleRate * 0.001))
        : 0.0;
    double releaseCoeff = (m_release > 0.0)
        ? qExp(-1.0 / (m_release * m_sampleRate * 0.001))
        : 0.0;
    double envAttack = qExp(-1.0 / (0.01 * m_sampleRate * 0.001));  ///< 快速包络攻击
    double envRelease = qExp(-1.0 / (50.0 * m_sampleRate * 0.001)); ///< 50ms包络释放

    /* 门关闭时的目标增益（线性值） */
    double rangeLin = qPow(10.0, m_range / 20.0);
    double thresholdLin = qPow(10.0, m_threshold / 20.0);

    /* 保持时间转换为采样数 */
    int holdSamples = static_cast<int>(m_hold * m_sampleRate * 0.001);

    bool prevGateOpen = m_gateOpen;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);

        /* 包络跟随器: 快速攻击，较慢释放 */
        if (absSample > m_envelope) {
            m_envelope = envAttack * m_envelope + (1.0 - envAttack) * absSample;
        } else {
            m_envelope = envRelease * m_envelope + (1.0 - envRelease) * absSample;
        }

        /* 门状态判定 */
        bool aboveThreshold = (m_envelope > thresholdLin);

        if (aboveThreshold) {
            /* 信号超过阈值，门应该打开 */
            m_holdCount = 0;
            m_gateOpen = true;
        } else if (m_gateOpen) {
            /* 信号低于阈值但门仍打开 */
            m_holdCount++;
            if (m_holdCount > holdSamples) {
                /* 保持时间结束，门开始关闭 */
                m_gateOpen = false;
            }
        }

        /* 增益平滑（攻击/释放） */
        double targetGain = m_gateOpen ? 1.0 : rangeLin;

        if (targetGain > m_gain) {
            /* 增益增加: 使用攻击时间 */
            m_gain = attackCoeff * m_gain + (1.0 - attackCoeff) * targetGain;
        } else {
            /* 增益减少: 使用释放时间 */
            m_gain = releaseCoeff * m_gain + (1.0 - releaseCoeff) * targetGain;
        }

        /* 应用增益 */
        output[i] = input[i] * m_gain;
    }

    /* 检测门状态变化 */
    if (prevGateOpen != m_gateOpen) {
        emit gateStateChanged(m_gateOpen);
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalProcessCalls++;
    m_stats.totalSamples += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessCalls;

    return output;
}

/**
 * @brief 重置所有统计计数器
 */
void NoiseGate3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

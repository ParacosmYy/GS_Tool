/**
 * @file GateExpand3.cpp
 * @brief 扩展噪声门处理器实现
 *
 * 实现带有attack/release/hold参数和可调范围的扩展噪声门。
 * 支持平滑的增益包络跟随，避免咔嗒噪声。
 * 当信号电平低于阈值时，增益按range参数进行衰减；
 * 当信号电平高于阈值时，增益平滑恢复到1.0。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/dsp59/GateExpand3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化噪声门处理器
 * @param parent 父QObject指针
 */
GateExpand3::GateExpand3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置门限阈值 (dB)
 * @param thresh 阈值电平，信号低于此值时开始衰减 (默认 -40.0 dB)
 */
void GateExpand3::setThreshold(double thresh)
{
    m_threshold = thresh;
}

/**
 * @brief 设置启动时间
 * @param ms 启动时间，门打开时的增益上升速度 (默认 1.0 ms)
 *
 * 较小的attack值使门快速打开，较大的值使过渡更平滑
 */
void GateExpand3::setAttack(double ms)
{
    m_attack = qMax(0.1, ms);
}

/**
 * @brief 设置释放时间
 * @param ms 释放时间，门关闭时的增益下降速度 (默认 100.0 ms)
 *
 * 较大的release值使门缓慢关闭，减少截止噪声
 */
void GateExpand3::setRelease(double ms)
{
    m_release = qMax(0.1, ms);
}

/**
 * @brief 设置保持时间
 * @param ms 保持时间，信号低于阈值后增益保持的时间 (默认 50.0 ms)
 *
 * 保持时间防止信号在阈值附近快速开关门（抖动）
 */
void GateExpand3::setHold(double ms)
{
    m_hold = qMax(0.0, ms);
}

/**
 * @brief 设置衰减范围 (dB)
 * @param db 最大衰减量 (默认 -80.0 dB，负值)
 *
 * range = 0 表示不衰减，range = -80 表示最大衰减80dB
 */
void GateExpand3::setRange(double db)
{
    m_range = qBound(-120.0, db, 0.0);
}

/**
 * @brief 处理音频信号，应用噪声门
 *
 * 处理流程:
 * 1. 计算每个采样的电平 (dB)
 * 2. 与阈值比较，确定目标增益
 * 3. 应用hold计时器防止抖动
 * 4. 使用attack/release系数平滑增益变化
 * 5. 将增益转换为线性值并应用到信号
 *
 * @param input 输入音频采样
 * @return 经噪声门处理后的音频采样
 */
QVector<double> GateExpand3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    QVector<double> output(n);

    if (n == 0) {
        m_stats.totalProcessings++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;
        emit processingCompleted(0, 1.0);
        return output;
    }

    /* 计算attack/release的平滑系数 (基于44.1kHz采样率假设) */
    double sampleRate = 44100.0;
    double attackCoeff = qExp(-1.0 / (m_attack * 0.001 * sampleRate));
    double releaseCoeff = qExp(-1.0 / (m_release * 0.001 * sampleRate));

    /* 将range从dB转换为线性增益 */
    double rangeLinear = qPow(10.0, m_range / 20.0);

    /* hold计数器 (以采样为单位) */
    double holdSamples = m_hold * 0.001 * sampleRate;
    double holdCounter = 0.0;

    /* 门状态: true = 信号高于阈值 (门打开) */
    bool gateOpen = true;

    /* 目标增益: 1.0 = 完全通过, rangeLinear = 最大衰减 */
    double targetGain = 1.0;

    double gainSum = 0.0;

    for (int i = 0; i < n; ++i) {
        /* 计算当前采样的电平 (dB) */
        double absSample = qAbs(input[i]);
        double levelDb = 0.0;

        if (absSample < 1e-10) {
            levelDb = -120.0;
        } else {
            levelDb = 20.0 * qLn(absSample) / qLn(10.0);
        }

        /* 判断信号是否高于阈值 */
        bool aboveThreshold = (levelDb >= m_threshold);

        /* 更新门状态 */
        if (aboveThreshold) {
            /* 信号高于阈值，打开门 */
            gateOpen = true;
            holdCounter = holdSamples;
            targetGain = 1.0;
        } else if (holdCounter > 0.0) {
            /* 信号低于阈值但在hold时间内，保持门打开 */
            holdCounter -= 1.0;
            targetGain = 1.0;
        } else {
            /* 信号低于阈值且hold时间已过，关闭门 */
            gateOpen = false;
            targetGain = rangeLinear;
        }

        /* 平滑增益过渡 */
        if (m_currentGain < targetGain) {
            /* 增益上升: 使用attack系数 */
            m_currentGain = targetGain + attackCoeff * (m_currentGain - targetGain);
        } else {
            /* 增益下降: 使用release系数 */
            m_currentGain = targetGain + releaseCoeff * (m_currentGain - targetGain);
        }

        /* 限制增益范围 */
        m_currentGain = qBound(rangeLinear, m_currentGain, 1.0);

        /* 应用增益 */
        output[i] = input[i] * m_currentGain;
        gainSum += m_currentGain;
    }

    /* 更新统计 */
    m_stats.totalProcessings++;
    m_stats.totalSamples += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    double avgGain = (n > 0) ? gainSum / n : 1.0;
    emit processingCompleted(n, avgGain);

    return output;
}

/**
 * @brief 重置所有统计数据
 */
void GateExpand3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_currentGain = 1.0;
}

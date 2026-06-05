/**
 * @file PhaseLockedLoop.cpp
 * @brief 数字锁相环实现
 */

#include "PhaseLockedLoop.h"
#include <QElapsedTimer>
#include <cmath>

PhaseLockedLoop::PhaseLockedLoop(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

double PhaseLockedLoop::process(double inputSample)
{
    QElapsedTimer timer;
    timer.start();

    /* 生成VCO输出: 正弦波 */
    double vcoOutput = std::sin(m_vcoPhase);

    /* 相位检测器: 乘积检波 */
    double error = phaseDetector(inputSample, vcoOutput);

    /* 环路滤波器: 一阶低通 */
    m_filteredError = loopFilter(error);

    /* VCO: 更新相位 */
    double controlVoltage = m_filteredError;
    m_vcoPhase = vco(controlVoltage);

    /* 保存当前相位误差(归一化到 [-pi, pi]) */
    m_phaseError = std::atan2(std::sin(error), std::cos(error));

    /* 锁定检测: 连续低误差计数 */
    bool wasLocked = m_locked;
    if (std::abs(m_phaseError) < kLockPhaseThreshold) {
        m_lockCounter++;
        if (m_lockCounter >= kLockThreshold && !m_locked) {
            m_locked = true;
        }
    } else {
        m_lockCounter = 0;
        m_locked = false;
    }

    /* 锁定状态变化时发射信号 */
    if (m_locked != wasLocked) {
        m_stats.totalLocks++;
        emit lockChanged(m_locked);
    }

    /* 统计更新 */
    m_stats.totalSamples++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalSamples > 0) ? m_timeSum / m_stats.totalSamples : 0.0;

    return m_vcoPhase;
}

void PhaseLockedLoop::setFrequency(double freq)
{
    if (freq > 0.0) {
        m_centerFreq = freq;
        m_vcoFreq = freq;
    }
}

void PhaseLockedLoop::setBandwidth(double bw)
{
    /* 带宽系数限制在 (0.0, 1.0] */
    m_bandwidth = std::max(0.001, std::min(1.0, bw));
}

void PhaseLockedLoop::reset()
{
    m_vcoFreq = m_centerFreq;
    m_vcoPhase = 0.0;
    m_phaseError = 0.0;
    m_filteredError = 0.0;
    m_locked = false;
    m_lockCounter = 0;
}

void PhaseLockedLoop::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

double PhaseLockedLoop::phaseDetector(double input, double vcoOutput) const
{
    /* 乘积检波器: 输出包含和频与差频分量 */
    return input * vcoOutput;
}

double PhaseLockedLoop::loopFilter(double error)
{
    /* 一阶IIR低通滤波器: y[n] = alpha * error + (1-alpha) * y[n-1]
     * alpha由带宽系数决定 */
    double alpha = m_bandwidth * 0.5;
    return alpha * error + (1.0 - alpha) * m_filteredError;
}

double PhaseLockedLoop::vco(double controlVoltage)
{
    /* VCO: 相位增量 = 2*pi*freq/sr + K_vco * control
     * K_vco为压控灵敏度 */
    double Kvco = 2.0 * M_PI * m_centerFreq / kSampleRate;
    double phaseInc = Kvco + m_bandwidth * controlVoltage * 10.0;

    /* 累加相位并取模 */
    double newPhase = m_vcoPhase + phaseInc;
    while (newPhase > 2.0 * M_PI) newPhase -= 2.0 * M_PI;
    while (newPhase < 0.0) newPhase += 2.0 * M_PI;

    /* 更新VCO频率估计 */
    m_vcoFreq = std::abs(phaseInc) * kSampleRate / (2.0 * M_PI);

    return newPhase;
}

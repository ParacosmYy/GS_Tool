/**
 * @file CostasLoop.cpp
 * @brief Costas环实现 — 二阶数字锁相环
 */

#include "utils/costas/CostasLoop.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

CostasLoop::CostasLoop(QObject* parent)
    : QObject(parent)
{
    updateGains();
}

double CostasLoop::process(double iSample, double qSample)
{
    QElapsedTimer timer;
    timer.start();

    /* 下变频: 旋转输入采样点 */
    double cosP = std::cos(m_phase);
    double sinP = std::sin(m_phase);
    double iOut = iSample * cosP + qSample * sinP;
    double qOut = -iSample * sinP + qSample * cosP;

    /* 鉴相器: 误差 = I * Q (适用于BPSK/QPSK) */
    double error = iOut * qOut;

    /* 环路滤波器: PI控制 */
    m_freq  += m_beta * error;
    m_phase += m_alpha * error + m_freq;

    /* 相位归一化到 [-pi, pi] */
    while (m_phase > M_PI)  m_phase -= 2.0 * M_PI;
    while (m_phase < -M_PI) m_phase += 2.0 * M_PI;

    /* 限制频率偏移范围 */
    double maxFreq = 0.5 * M_PI;
    if (m_freq > maxFreq)  m_freq = maxFreq;
    if (m_freq < -maxFreq) m_freq = -maxFreq;

    /* 更新统计 */
    m_stats.totalSamples++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSamples;

    emit frequencyUpdated(m_freq);
    return iOut;
}

void CostasLoop::setBandwidth(double bw)
{
    m_bw = qBound(1e-6, bw, 1.0);
    updateGains();
}

void CostasLoop::reset()
{
    m_phase = 0.0;
    m_freq  = 0.0;
}

void CostasLoop::updateGains()
{
    /* 二阶环路阻尼系数 zeta = sqrt(2)/2 (临界阻尼) */
    double zeta = M_SQRT2 / 2.0;
    double wn   = m_bw; /* 自然频率 ≈ 带宽 */

    /* 数字PI增益 */
    double denom = 1.0 + 2.0 * zeta * wn + wn * wn;
    m_alpha = 4.0 * zeta * wn / denom;
    m_beta  = 4.0 * wn * wn / denom;
}

void CostasLoop::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

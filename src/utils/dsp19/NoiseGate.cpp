/**
 * @file NoiseGate.cpp
 * @brief 噪声门处理器实现 — 阈值检测/包络控制/侧链滤波
 */

#include "utils/dsp19/NoiseGate.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
NoiseGate::NoiseGate(QObject* parent)
    : QObject(parent)
    , m_threshold(0.1)
    , m_attackCoeff(0.0)
    , m_releaseCoeff(0.0)
    , m_holdTime(0.0)
    , m_hysteresis(0.02)
    , m_sidechainFreq(80.0)
    , m_lookahead(0)
    , m_envelope(0.0)
    , m_gateOpen(false)
    , m_holdCounter(0)
    , m_prevSidechain(0.0)
{
}

/** @brief 设置门控阈值(dB) @param threshold 阈值 */
void NoiseGate::setThreshold(double threshold)
{
    /* 将dB转换为线性值 */
    m_threshold = qPow(10.0, threshold / 20.0);
}

/** @brief 设置攻击时间(ms) @param attack 攻击时间 */
void NoiseGate::setAttack(double attack)
{
    double ms = qMax(0.01, attack);
    m_attackCoeff = 1.0 - qExp(-1.0 / (ms * 0.001 * 44100.0));
}

/** @brief 设置释放时间(ms) @param release 释放时间 */
void NoiseGate::setRelease(double release)
{
    double ms = qMax(0.01, release);
    m_releaseCoeff = 1.0 - qExp(-1.0 / (ms * 0.001 * 44100.0));
}

/** @brief 设置保持时间(ms) @param hold 保持时间 */
void NoiseGate::setHold(double hold)
{
    m_holdTime = hold * 44100.0 / 1000.0;
}

/** @brief 设置滞回宽度(dB) @param hysteresis 滞回宽度 */
void NoiseGate::setHysteresis(double hysteresis)
{
    m_hysteresis = qPow(10.0, hysteresis / 20.0);
}

/** @brief 设置侧链低截止频率 @param freq 频率 */
void NoiseGate::setSidechainLowCut(double freq)
{
    m_sidechainFreq = qMax(20.0, freq);
}

/** @brief 设置前瞻采样数 @param samples 采样数 */
void NoiseGate::setLookahead(int samples)
{
    m_lookahead = qMax(0, samples);
}

/** @brief 处理音频块 @param input 输入采样 @param sampleRate 采样率 @return 处理后采样 */
QVector<double> NoiseGate::process(const QVector<double>& input,
                                   double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return {};

    QVector<double> output(n, 0.0);

    /* 计算攻击/释放系数 */
    double attackCoeff = (m_attackCoeff > 0) ? m_attackCoeff
        : 1.0 - qExp(-1.0 / (1.0 * 0.001 * sampleRate));
    double releaseCoeff = (m_releaseCoeff > 0) ? m_releaseCoeff
        : 1.0 - qExp(-1.0 / (50.0 * 0.001 * sampleRate));

    /* 前瞻缓冲区 */
    QVector<double> lookaheadBuf(m_lookahead, 0.0);
    int writePos = 0;
    int readPos = 0;

    int gatedCount = 0;

    for (int i = 0; i < n; ++i) {
        double sample = input[i];

        /* 侧链滤波: 一阶高通 */
        double sc = simpleHighpass(sample);

        /* 检测阈值(带滞回) */
        double absSc = qAbs(sc);
        bool aboveThreshold;
        if (m_gateOpen) {
            aboveThreshold = absSc > (m_threshold - m_hysteresis);
        } else {
            aboveThreshold = absSc > (m_threshold + m_hysteresis);
        }

        /* 更新包络 */
        if (aboveThreshold) {
            m_envelope += attackCoeff * (absSc - m_envelope);
            m_holdCounter = static_cast<int>(m_holdTime);
            if (!m_gateOpen) {
                m_gateOpen = true;
                emit gateStateChanged(true);
            }
        } else {
            if (m_holdCounter > 0) {
                --m_holdCounter;
            } else {
                m_envelope += releaseCoeff * (0.0 - m_envelope);
                if (m_gateOpen && m_envelope < m_threshold * 0.01) {
                    m_gateOpen = false;
                    emit gateStateChanged(false);
                }
            }
        }

        /* 应用增益 */
        double gain = qMin(1.0, m_envelope / qMax(m_threshold, 1e-10));
        gain = qBound(0.0, gain, 1.0);

        /* 前瞻处理 */
        if (m_lookahead > 0) {
            lookaheadBuf[writePos] = sample * gain;
            output[i] = lookaheadBuf[readPos];
            writePos = (writePos + 1) % m_lookahead;
            readPos = (readPos + 1) % m_lookahead;
        } else {
            output[i] = sample * gain;
        }

        if (gain < 0.01) ++gatedCount;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalBlocksProcessed;
    m_stats.totalSamplesProcessed += n;
    m_stats.totalSamplesGated += gatedCount;
    m_stats.gateRatio = (m_stats.totalSamplesProcessed > 0)
        ? static_cast<double>(m_stats.totalSamplesGated)
          / m_stats.totalSamplesProcessed : 0.0;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBlocksProcessed);

    emit blockProcessed(n, gatedCount);
    return output;
}

/** @brief 检测信号是否超过阈值 @param sample 采样 @return 是否超过 */
bool NoiseGate::detectThreshold(double sample)
{
    double sc = simpleHighpass(sample);
    return qAbs(sc) > m_threshold;
}

/** @brief 获取当前包络增益 @return 增益(0-1) */
double NoiseGate::currentEnvelope() const
{
    return m_envelope;
}

/** @brief 应用侧链高通滤波 @param samples 输入 @param sampleRate 采样率 @return 滤波后数据 */
QVector<double> NoiseGate::applySidechainFilter(
    const QVector<double>& samples, double sampleRate)
{
    int n = samples.size();
    QVector<double> filtered(n);
    double rc = 1.0 / (2.0 * M_PI * m_sidechainFreq);
    double dt = 1.0 / sampleRate;
    double alpha = rc / (rc + dt);
    double prev = m_prevSidechain;

    for (int i = 0; i < n; ++i) {
        filtered[i] = alpha * prev + alpha * (samples[i] - prev);
        prev = filtered[i];
    }
    m_prevSidechain = prev;
    return filtered;
}

/** @brief 重置统计 */
void NoiseGate::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope = 0.0;
    m_gateOpen = false;
    m_holdCounter = 0;
    m_prevSidechain = 0.0;
}

/** @brief 简单一阶高通滤波 @param sample 输入 @return 滤波后 */
double NoiseGate::simpleHighpass(double sample)
{
    double rc = 1.0 / (2.0 * M_PI * m_sidechainFreq);
    double dt = 1.0 / 44100.0;
    double alpha = rc / (rc + dt);
    double output = alpha * m_prevSidechain + alpha * (sample - m_prevSidechain);
    m_prevSidechain = output;
    return output;
}

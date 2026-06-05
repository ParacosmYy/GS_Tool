/**
 * @file EnvelopeFollower.cpp
 * @brief 信号包络跟踪器实现 — 攻击/释放参数控制
 */

#include "utils/signal8/EnvelopeFollower.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

EnvelopeFollower::EnvelopeFollower(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    configure(Config{});
}

void EnvelopeFollower::configure(const Config& config)
{
    m_config = config;

    /* 预计算IIR系数 */
    m_attackCoeff = calcAttackCoeff(config.attackTimeMs, config.sampleRate);
    m_releaseCoeff = calcReleaseCoeff(config.releaseTimeMs, config.sampleRate);

    m_envelope = 0.0;
    m_rmsAccum = 0.0;
    m_holdValue = 0.0;
    m_holdCounter = 0;
}

double EnvelopeFollower::processSample(double sample)
{
    double result = 0.0;

    switch (m_config.mode) {
    case Mode::Peak:
    case Mode::LogPeak:
        result = updatePeak(qAbs(sample));
        break;
    case Mode::RMS:
        result = updateRMS(sample);
        break;
    case Mode::PeakHold:
        result = updatePeakHold(qAbs(sample));
        break;
    }

    ++m_stats.totalSamplesProcessed;

    /* 更新统计 */
    if (result > m_stats.peakEnvelope) {
        m_stats.peakEnvelope = result;
    }
    if (result < m_stats.minEnvelope) {
        m_stats.minEnvelope = result;
    }

    return result;
}

EnvelopeFollower::EnvelopeResult EnvelopeFollower::processBuffer(
    const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    EnvelopeResult result;

    if (samples.isEmpty()) return result;

    result.envelope.reserve(samples.size());
    double sumSq = 0.0;
    double peak = 0.0;

    for (int i = 0; i < samples.size(); ++i) {
        double env = processSample(samples[i]);
        result.envelope.append(env);
        sumSq += samples[i] * samples[i];
        if (qAbs(samples[i]) > peak) peak = qAbs(samples[i]);
    }

    result.peakValue = peak;
    result.rmsValue = qSqrt(sumSq / samples.size());
    result.crestFactor = (result.rmsValue > 1e-15)
        ? peak / result.rmsValue : 0.0;

    /* 动态范围(dB) */
    double minEnv = *std::min_element(result.envelope.begin(),
                                       result.envelope.end());
    result.dynamicRange = (minEnv > 1e-15 && peak > 1e-15)
        ? 20.0 * qLog10(peak / minEnv) : 0.0;

    /* 更新统计 */
    ++m_stats.totalBuffersProcessed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuffersProcessed;

    emit bufferProcessed(result.peakValue, result.rmsValue);
    return result;
}

QVector<int> EnvelopeFollower::detectAboveThreshold(
    double threshold, const QVector<double>& samples)
{
    QVector<int> positions;

    for (int i = 0; i < samples.size(); ++i) {
        double env = processSample(samples[i]);
        if (env > threshold) {
            positions.append(i);
            emit thresholdExceeded(env, threshold);
        }
    }

    return positions;
}

double EnvelopeFollower::currentEnvelope() const
{
    return m_envelope;
}

void EnvelopeFollower::reset()
{
    m_envelope = 0.0;
    m_rmsAccum = 0.0;
    m_holdValue = 0.0;
    m_holdCounter = 0;
}

double EnvelopeFollower::calcAttackCoeff(double attackTimeMs,
                                          double sampleRate)
{
    if (attackTimeMs <= 0.0 || sampleRate <= 0.0) return 1.0;
    double tau = attackTimeMs / 1000.0 * sampleRate;
    return 1.0 - qExp(-1.0 / qMax(tau, 1e-10));
}

double EnvelopeFollower::calcReleaseCoeff(double releaseTimeMs,
                                           double sampleRate)
{
    if (releaseTimeMs <= 0.0 || sampleRate <= 0.0) return 1.0;
    double tau = releaseTimeMs / 1000.0 * sampleRate;
    return 1.0 - qExp(-1.0 / qMax(tau, 1e-10));
}

EnvelopeFollower::Config EnvelopeFollower::config() const
{
    return m_config;
}

EnvelopeFollower::Stats EnvelopeFollower::stats() const
{
    return m_stats;
}

void EnvelopeFollower::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

double EnvelopeFollower::updatePeak(double input)
{
    /* 攻击/释放分离的包络跟踪 */
    double prevEnvelope = m_envelope;

    if (input > m_envelope) {
        /* 信号上升: 使用攻击系数 */
        m_envelope += m_attackCoeff * (input - m_envelope);
        if (m_envelope > prevEnvelope) {
            ++m_stats.totalAttacks;
        }
    } else {
        /* 信号下降: 使用释放系数 */
        m_envelope += m_releaseCoeff * (input - m_envelope);
        if (m_envelope < prevEnvelope) {
            ++m_stats.totalReleases;
        }
    }

    /* 对数模式: 转换为dB */
    if (m_config.mode == Mode::LogPeak) {
        return (m_envelope > 1e-15)
            ? 20.0 * qLn(m_envelope) / qLn(10.0) : -300.0;
    }

    return m_envelope;
}

double EnvelopeFollower::updateRMS(double input)
{
    /* RMS包络: 平滑平方值 */
    double inputSq = input * input;
    double coeff = (inputSq > m_rmsAccum) ? m_attackCoeff : m_releaseCoeff;

    m_rmsAccum += coeff * (inputSq - m_rmsAccum);
    m_rmsAccum = qMax(0.0, m_rmsAccum);  /* 防止负数 */

    m_envelope = qSqrt(m_rmsAccum);
    return m_envelope;
}

double EnvelopeFollower::updatePeakHold(double input)
{
    int holdSamples = static_cast<int>(
        m_config.holdTimeMs * m_config.sampleRate / 1000.0);

    if (input >= m_holdValue) {
        /* 新峰值: 直接更新 */
        m_holdValue = input;
        m_holdCounter = holdSamples;
        m_envelope = m_holdValue;
    } else if (m_holdCounter > 0) {
        /* 保持阶段 */
        --m_holdCounter;
        m_envelope = m_holdValue;
    } else {
        /* 衰减阶段 */
        m_holdValue += m_releaseCoeff * (input - m_holdValue);
        m_envelope = m_holdValue;
        ++m_stats.totalReleases;
    }

    return m_envelope;
}

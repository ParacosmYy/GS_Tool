/**
 * @file Oscillator.cpp
 * @brief 多波形振荡器实现 — BLEP抗混叠 + 调频/调幅合成
 */

#include "utils/dsp17/Oscillator.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cstring>

/* ──────────────────── 构造/析构 ──────────────────── */

/** @brief 构造函数 @param parent 父对象 */
Oscillator::Oscillator(QObject* parent)
    : QObject(parent)
    , m_phaseAccum(0.0)
    , m_phaseInc(0.0)
{
    std::memset(m_blepHistorySaw, 0, sizeof(m_blepHistorySaw));
    std::memset(m_blepHistorySq, 0, sizeof(m_blepHistorySq));
    updatePhaseInc();
}

/** @brief 析构函数 */
Oscillator::~Oscillator() = default;

/* ──────────────────── 参数控制 ──────────────────── */

/** @brief 设置振荡器参数 @param params 参数 */
void Oscillator::setParameters(const Parameters& params)
{
    bool freqChanged = !qFuzzyCompare(m_params.frequency, params.frequency);
    bool waveChanged = (m_params.waveform != params.waveform);

    m_params = params;
    updatePhaseInc();

    if (freqChanged) {
        ++m_stats.totalFrequencyChanges;
        emit frequencyChanged(m_params.frequency);
    }
    if (waveChanged) {
        ++m_stats.totalWaveformChanges;
        emit waveformChanged(static_cast<int>(m_params.waveform));
    }
}

/** @brief 获取当前参数 @return 参数 */
Oscillator::Parameters Oscillator::parameters() const
{
    return m_params;
}

/** @brief 设置频率 @param freq 频率(Hz) */
void Oscillator::setFrequency(double freq)
{
    if (!qFuzzyCompare(m_params.frequency, freq)) {
        m_params.frequency = freq;
        updatePhaseInc();
        ++m_stats.totalFrequencyChanges;
        emit frequencyChanged(freq);
    }
}

/** @brief 设置振幅 @param amp 振幅[0,1] */
void Oscillator::setAmplitude(double amp)
{
    m_params.amplitude = qBound(0.0, amp, 1.0);
}

/** @brief 设置波形类型 @param wf 波形 */
void Oscillator::setWaveform(Waveform wf)
{
    if (m_params.waveform != wf) {
        m_params.waveform = wf;
        ++m_stats.totalWaveformChanges;
        emit waveformChanged(static_cast<int>(wf));
    }
}

/** @brief 更新相位增量 */
void Oscillator::updatePhaseInc()
{
    if (m_params.sampleRate > 0) {
        m_phaseInc = m_params.frequency / m_params.sampleRate;
    }
}

/* ──────────────────── 信号生成 ──────────────────── */

/** @brief 生成单个采样点 @return 采样值[-1,1] */
double Oscillator::tick()
{
    double sample = 0.0;
    double phase = m_phaseAccum;

    switch (m_params.waveform) {
    case Sine:
        sample = qSin(2.0 * M_PI * phase);
        break;

    case Square:
        if (m_params.enableAntiAliasing) {
            double inc = m_phaseInc;
            double raw = phase < 0.5 ? 1.0 : -1.0;
            sample = raw + blepSquare(phase, inc);
        } else {
            sample = phase < 0.5 ? 1.0 : -1.0;
        }
        break;

    case Sawtooth:
        if (m_params.enableAntiAliasing) {
            double inc = m_phaseInc;
            double raw = 2.0 * phase - 1.0;
            sample = raw + blepSaw(phase, inc);
        } else {
            sample = 2.0 * phase - 1.0;
        }
        break;

    case Triangle:
        sample = phase < 0.5
            ? (4.0 * phase - 1.0)
            : (3.0 - 4.0 * phase);
        break;

    case Pulse:
        sample = phase < m_params.pulseWidth ? 1.0 : -1.0;
        if (m_params.enableAntiAliasing) {
            /* 脉冲波 = 方波在非50%占空比的推广，简化处理 */
            double inc = m_phaseInc;
            double raw = phase < m_params.pulseWidth ? 1.0 : -1.0;
            sample = raw + blepSquare(phase, inc) * 0.5;
        }
        break;
    }

    /* 幅度调制 */
    double amp = m_params.amplitude;
    if (m_params.enableAM && m_params.amRate > 0) {
        static double amPhase = 0.0;
        double amMod = 1.0 - m_params.amDepth
            + m_params.amDepth * qSin(2.0 * M_PI * amPhase);
        amp *= amMod;
        amPhase += m_params.amRate / m_params.sampleRate;
        if (amPhase >= 1.0) amPhase -= 1.0;
    }

    /* 更新相位累加器 */
    double freq = m_params.frequency;
    if (m_params.enableFM && m_params.fmRate > 0) {
        static double fmPhase = 0.0;
        double fmMod = m_params.fmDepth * qSin(2.0 * M_PI * fmPhase);
        freq += fmMod;
        fmPhase += m_params.fmRate / m_params.sampleRate;
        if (fmPhase >= 1.0) fmPhase -= 1.0;
    }

    m_phaseAccum += freq / m_params.sampleRate;
    while (m_phaseAccum >= 1.0) m_phaseAccum -= 1.0;
    while (m_phaseAccum < 0.0) m_phaseAccum += 1.0;

    ++m_stats.totalSamplesGenerated;
    return sample * amp;
}

/** @brief 生成指定数量的采样点 @param numSamples 采样数 @return 采样缓冲区 */
QVector<double> Oscillator::generate(int numSamples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> buffer;
    buffer.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        buffer.append(tick());
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalSamplesGenerated, 1ULL));

    emit generationCompleted(numSamples);
    return buffer;
}

/** @brief 使用频率调制生成采样 @param numSamples 采样数 @param modSignal 调制信号 @return 采样缓冲区 */
QVector<double> Oscillator::generateWithFM(int numSamples,
                                           const QVector<double>& modSignal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> buffer;
    buffer.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        double mod = (i < modSignal.size()) ? modSignal[i] : 0.0;
        double freq = m_params.frequency + mod;
        double phase = m_phaseAccum;

        double sample = 0.0;
        switch (m_params.waveform) {
        case Sine:
            sample = qSin(2.0 * M_PI * phase);
            break;
        case Square:
            sample = phase < 0.5 ? 1.0 : -1.0;
            break;
        case Sawtooth:
            sample = 2.0 * phase - 1.0;
            break;
        case Triangle:
            sample = phase < 0.5 ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
            break;
        case Pulse:
            sample = phase < m_params.pulseWidth ? 1.0 : -1.0;
            break;
        }

        m_phaseAccum += freq / m_params.sampleRate;
        while (m_phaseAccum >= 1.0) m_phaseAccum -= 1.0;

        buffer.append(sample * m_params.amplitude);
    }

    m_stats.totalSamplesGenerated += static_cast<quint64>(numSamples);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalSamplesGenerated, 1ULL));

    emit generationCompleted(numSamples);
    return buffer;
}

/* ──────────────────── 状态控制 ──────────────────── */

/** @brief 重置相位累加器 */
void Oscillator::resetPhase()
{
    m_phaseAccum = 0.0;
}

/** @brief 设置当前相位 @param phase 相位[0,1) */
void Oscillator::setPhase(double phase)
{
    m_phaseAccum = qBound(0.0, phase, 1.0);
}

/** @brief 获取当前相位 @return 相位[0,1) */
double Oscillator::currentPhase() const
{
    return m_phaseAccum;
}

/* ──────────────────── 原始波形采样 ──────────────────── */

/** @brief 原始波形采样(无抗混叠) @param phase 相位 @return 采样值 */
double Oscillator::rawSample(double phase) const
{
    switch (m_params.waveform) {
    case Sine:     return qSin(2.0 * M_PI * phase);
    case Square:   return phase < 0.5 ? 1.0 : -1.0;
    case Sawtooth: return 2.0 * phase - 1.0;
    case Triangle: return phase < 0.5 ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
    case Pulse:    return phase < m_params.pulseWidth ? 1.0 : -1.0;
    }
    return 0.0;
}

/* ──────────────────── BLEP抗混叠 ──────────────────── */

/** @brief BLEP修正(方波) @param phase 当前相位 @param inc 相位增量 @return BLEP修正量 */
double Oscillator::blepSquare(double phase, double inc) const
{
    double correction = 0.0;

    /* 方波在phase=0.5处有一个下降跳变 */
    double t = phase - 0.5;
    if (qAbs(t) < inc * BLEP_HISTORY) {
        double scaled = t / inc;
        correction = blepIntegral(scaled);
    }

    /* phase=0处有一个上升跳变 */
    t = phase;
    if (qAbs(t) < inc * BLEP_HISTORY) {
        double scaled = t / inc;
        correction -= blepIntegral(scaled);
    }

    return correction;
}

/** @brief BLEP修正(锯齿波) @param phase 当前相位 @param inc 相位增量 @return BLEP修正量 */
double Oscillator::blepSaw(double phase, double inc) const
{
    double correction = 0.0;

    /* 锯齿波在phase=0处有一个从+1到-1的跳变 */
    double t = phase;
    if (qAbs(t) < inc * BLEP_HISTORY) {
        double scaled = t / inc;
        correction = -blepIntegral(scaled);
    }

    return correction * 2.0; /* 锯齿波跳变为2(-1到+1) */
}

/** @brief BLEP积分(近似sinc积分) @param t 归一化时间 @return 积分值 */
double Oscillator::blepIntegral(double t) const
{
    /* 多项式近似: 近似 ∫sinc(x)dx 在(-inf, t) 的积分 */
    /* 使用N-point BLEP的简化近似 */
    double t2 = t * t;
    double t3 = t2 * t;

    if (t < -1.0) return 0.0;
    if (t > 1.0)  return 1.0;

    /* 三次多项式近似 */
    return 0.5 + t * (0.5 - t2 / 6.0);
}

/* ──────────────────── 统计 ──────────────────── */

/** @brief 获取统计 @return 统计 */
Oscillator::Stats Oscillator::stats() const { return m_stats; }

/** @brief 重置统计 */
void Oscillator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

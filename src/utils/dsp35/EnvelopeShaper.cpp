/**
 * @file EnvelopeShaper.cpp
 * @brief ADSR 包络整形器实现 — 攻击/衰减/持续/释放包络与包络跟随器
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 ADSR 包络生成和包络跟随器功能。
 * 支持 gate 信号驱动的状态机（Attack->Decay->Sustain->Release）
 * 以及基于时间常数的包络跟随模式。
 */

#include "utils/dsp35/EnvelopeShaper.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief ADSR 状态枚举
 *
 * 状态转换：
 * - Idle: 等待 gate 信号
 * - Attack: 从 0 上升到峰值（1.0），到达后转入 Decay
 * - Decay: 从峰值下降到 Sustain 电平，到达后转入 Sustain
 * - Sustain: 维持在 Sustain 电平，直到 gate 释放
 * - Release: 从当前电平衰减到 0，到达后转入 Idle
 */
enum ADSRState {
    State_Idle = 0,
    State_Attack,
    State_Decay,
    State_Sustain,
    State_Release
};

/**
 * @brief 构造函数，初始化默认 ADSR 参数
 * @param parent 父对象
 */
EnvelopeShaper::EnvelopeShaper(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("EnvelopeShaper"));
}

/**
 * @brief 重置统计信息
 */
void EnvelopeShaper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设置 Attack 阶段时间
 * @param ms 毫秒数，从 0 上升到峰值的时间
 */
void EnvelopeShaper::setAttack(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置 Decay 阶段时间
 * @param ms 毫秒数，从峰值下降到 Sustain 电平的时间
 */
void EnvelopeShaper::setDecay(double ms)
{
    m_decay = qMax(0.01, ms);
}

/**
 * @brief 设置 Sustain 电平
 * @param level 持续电平 [0.0, 1.0]
 */
void EnvelopeShaper::setSustain(double level)
{
    m_sustain = qBound(0.0, level, 1.0);
}

/**
 * @brief 设置 Release 阶段时间
 * @param ms 毫秒数，从 Sustain 电平释放到 0 的时间
 */
void EnvelopeShaper::setRelease(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置采样率
 * @param rate 采样率 (Hz)，用于将时间参数转换为样本数
 */
void EnvelopeShaper::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 处理单个样本，返回包络值
 *
 * 使用基于时间常数的包络跟随算法：
 * - 当输入信号大于当前包络值时，使用 attack 时间常数上升
 * - 当输入信号小于当前包络值时，使用 release 时间常数下降
 *
 * attack/release 系数由一阶 IIR 低通滤波器公式推导：
 * coeff = exp(-1.0 / (time_ms * sampleRate * 0.001))
 *
 * @param sample 输入音频样本
 * @return 当前包络值
 */
double EnvelopeShaper::processOne(double sample)
{
    const double absSample = qFabs(sample);

    // 计算包络跟随器系数
    // attack 时间常数控制上升速度
    // release 时间常数控制下降速度
    const double attackCoeff = qExp(-1.0 / (m_attack * m_sampleRate * 0.001));
    const double releaseCoeff = qExp(-1.0 / (m_release * m_sampleRate * 0.001));

    // 包络跟随：对绝对值进行一阶低通滤波
    if (absSample > m_envelope) {
        // 信号上升阶段：使用 attack 系数，快速跟踪峰值
        m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * absSample;
    } else {
        // 信号下降阶段：使用 release 系数，缓慢衰减
        m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * absSample;
    }

    // 防止包络值溢出或下溢
    m_envelope = qBound(0.0, m_envelope, 1.0);

    return m_envelope;
}

/**
 * @brief 基于 ADSR 状态机处理单个样本
 *
 * 完整的 ADSR 包络状态机：
 * 1. 当输入绝对值超过 gate 阈值时触发 Attack
 * 2. Attack 阶段线性上升到 1.0，速率 = 1.0 / (attackTime * sampleRate)
 * 3. Decay 阶段线性下降到 sustain 电平，速率 = (1.0 - sustain) / (decayTime * sampleRate)
 * 4. Sustain 阶段保持 sustain 电平
 * 5. gate 释放后进入 Release，线性下降到 0
 *
 * @param sample 输入音频样本
 * @param state 当前 ADSR 状态（引用，会被更新）
 * @return ADSR 包络值
 */
static double processADSR(double sample, int &state,
                           double attack, double decay,
                           double sustain, double release,
                           double sampleRate, double currentEnv)
{
    const double gateThreshold = 0.001;
    const bool gateOn = qFabs(sample) > gateThreshold;
    double env = currentEnv;

    switch (state) {
    case State_Idle:
        if (gateOn) {
            state = State_Attack;
        }
        break;
    case State_Attack: {
        // 线性上升，每样本增量
        const double rate = 1.0 / (attack * sampleRate * 0.001);
        env += rate;
        if (env >= 1.0) {
            env = 1.0;
            state = State_Decay;
        }
        if (!gateOn) {
            state = State_Release;
        }
        break;
    }
    case State_Decay: {
        // 线性下降到 sustain 电平
        const double rate = (1.0 - sustain) / (decay * sampleRate * 0.001);
        env -= rate;
        if (env <= sustain) {
            env = sustain;
            state = State_Sustain;
        }
        if (!gateOn) {
            state = State_Release;
        }
        break;
    }
    case State_Sustain:
        env = sustain;
        if (!gateOn) {
            state = State_Release;
        }
        break;
    case State_Release: {
        // 线性下降到 0
        const double rate = env / qMax(1.0, release * sampleRate * 0.001);
        env -= rate;
        if (env <= 0.0) {
            env = 0.0;
            state = State_Idle;
        }
        if (gateOn) {
            state = State_Attack;
        }
        break;
    }
    }

    return qBound(0.0, env, 1.0);
}

/**
 * @brief 批量处理音频数据，计算完整包络
 *
 * 对输入信号逐样本执行包络跟随，并将结果缓存。
 * 同时计算 ADSR 状态机驱动的包络作为第二层处理。
 * 处理完成后发射 processingComplete 信号。
 *
 * @param input 输入音频样本序列
 * @return 包络值序列
 */
QVector<double> EnvelopeShaper::process(const QVector<double> &input)
{
    QElapsedTimer timer;
    timer.start();

    const int n = input.size();
    m_envelopeBuf.resize(n);

    // 初始化 ADSR 状态
    int adsrState = State_Idle;
    double adsrEnv = 0.0;

    for (int i = 0; i < n; ++i) {
        // 第一层：包络跟随器
        const double followerEnv = processOne(input[i]);

        // 第二层：ADSR 状态机包络
        adsrEnv = processADSR(input[i], adsrState,
                               m_attack, m_decay, m_sustain, m_release,
                               m_sampleRate, adsrEnv);

        // 混合两层包络：取包络跟随器值乘以 ADSR 门控
        m_envelopeBuf[i] = followerEnv * adsrEnv;
    }

    // 更新统计信息
    m_stats.totalSamplesProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1.0, static_cast<double>(m_stats.totalSamplesProcessed));

    emit processingComplete(n);
    return m_envelopeBuf;
}

/**
 * @brief 获取最近一次处理的包络数据
 * @return 包络值序列的副本
 */
QVector<double> EnvelopeShaper::envelope() const
{
    return m_envelopeBuf;
}

/**
 * @brief 重置包络状态到初始值
 *
 * 清除包络缓冲区，将包络值归零。
 * 不影响 ADSR 参数设置和统计信息。
 */
void EnvelopeShaper::reset()
{
    m_envelope = 0.0;
    m_envelopeBuf.clear();
}

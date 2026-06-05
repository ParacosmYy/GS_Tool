/**
 * @file ChorusEffect.cpp
 * @brief 合唱/镶边效果器实现 — 多延迟线/LFO调制/立体声展宽
 */

#include "utils/dsp21/ChorusEffect.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ChorusEffect::ChorusEffect(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_bufferSize(0)
    , m_lfoPhase(0.0)
    , m_initialized(false)
    , m_timeSum(0.0)
{
}

void ChorusEffect::setParams(const ChorusParams& params)
{
    m_params = params;
    /* 限制反馈防止自激 */
    m_params.feedback = qBound(0.0, m_params.feedback, 0.95);
    m_params.mix = qBound(0.0, m_params.mix, 1.0);
    m_params.stereoSpread = qBound(0.0, m_params.stereoSpread, 1.0);
    m_params.voiceCount = qBound(1, m_params.voiceCount, 8);
}

/** @brief 初始化处理器 @param sampleRate 采样率 @param maxDelay 最大延迟 */
void ChorusEffect::initialize(double sampleRate, double maxDelay)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_bufferSize = static_cast<int>(maxDelay * m_sampleRate) + 1;
    m_bufferSize = qMax(m_bufferSize, 64);

    int voices = m_params.voiceCount;
    m_delayBuffers.resize(voices);
    m_writePos.resize(voices, 0);

    for (int i = 0; i < voices; ++i) {
        m_delayBuffers[i].resize(m_bufferSize, 0.0);
        m_writePos[i] = 0;
    }

    m_lfoPhase = 0.0;
    m_initialized = true;
}

/** @brief 处理单采样 @param input 输入 @return 输出 */
double ChorusEffect::processSample(double input)
{
    if (!m_initialized) return input;

    int voices = m_params.voiceCount;
    double wetSum = 0.0;

    for (int v = 0; v < voices; ++v) {
        /* 计算当前LFO调制的延迟量 */
        double lfoVal = lfoValue(v);
        double modDelay = m_params.baseDelay * m_sampleRate
            + lfoVal * m_params.depth * m_sampleRate;

        /* 从延迟线读取 */
        double delayed = readDelay(v, modDelay);

        /* 加入反馈 */
        double feedbackSample = input + delayed * m_params.feedback;

        /* 写入延迟线 */
        writeDelay(v, feedbackSample);

        wetSum += delayed;
    }

    /* 归一化多声部 */
    double wet = wetSum / static_cast<double>(voices);
    double output = input * (1.0 - m_params.mix) + wet * m_params.mix;

    /* 推进LFO相位 */
    double phaseInc = m_params.rate / m_sampleRate;
    m_lfoPhase += phaseInc;
    if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;

    /* 软削波 */
    output = qBound(-1.0, output, 1.0);

    /* 统计更新 */
    ++m_stats.totalSamplesProcessed;
    double absOut = qAbs(output);
    if (absOut > m_stats.peakAmplitude) {
        m_stats.peakAmplitude = absOut;
    }

    return output;
}

/** @brief 处理单声道块 @param input 输入块 @return 处理后块 */
QVector<double> ChorusEffect::processBlock(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processSample(input[i]);
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalBlocksProcessed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBlocksProcessed);

    emit blockProcessed(input.size());
    return output;
}

/** @brief 处理立体声块 @param left 左声道 @param right 右声道 @return (左,右) */
QPair<QVector<double>, QVector<double>> ChorusEffect::processStereo(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(left.size(), right.size());
    QVector<double> outL(n), outR(n);
    int voices = m_params.voiceCount;
    double spread = m_params.stereoSpread;

    for (int i = 0; i < n; ++i) {
        double mono = (left[i] + right[i]) * 0.5;
        double wetL = 0.0, wetR = 0.0;

        for (int v = 0; v < voices; ++v) {
            double lfoVal = lfoValue(v);
            double modDelay = m_params.baseDelay * m_sampleRate
                + lfoVal * m_params.depth * m_sampleRate;
            double delayed = readDelay(v, modDelay);
            double fbSample = mono + delayed * m_params.feedback;
            writeDelay(v, fbSample);

            /* 立体声展宽: 奇数声部反相给右声道 */
            if (v % 2 == 0) {
                wetL += delayed * (1.0 + spread);
                wetR += delayed * (1.0 - spread);
            } else {
                wetL += delayed * (1.0 - spread);
                wetR += delayed * (1.0 + spread);
            }
        }

        wetL /= voices;
        wetR /= voices;

        outL[i] = left[i] * (1.0 - m_params.mix) + wetL * m_params.mix;
        outR[i] = right[i] * (1.0 - m_params.mix) + wetR * m_params.mix;

        outL[i] = qBound(-1.0, outL[i], 1.0);
        outR[i] = qBound(-1.0, outR[i], 1.0);

        double phaseInc = m_params.rate / m_sampleRate;
        m_lfoPhase += phaseInc;
        if (m_lfoPhase >= 1.0) m_lfoPhase -= 1.0;

        ++m_stats.totalSamplesProcessed;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalBlocksProcessed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBlocksProcessed);

    return {outL, outR};
}

/** @brief 重置延迟线和LFO */
void ChorusEffect::reset()
{
    for (int v = 0; v < m_delayBuffers.size(); ++v) {
        m_delayBuffers[v].fill(0.0);
        m_writePos[v] = 0;
    }
    m_lfoPhase = 0.0;
}

/** @brief 计算LFO值 @param voiceIndex 声部索引 @return LFO值[-1,1] */
double ChorusEffect::lfoValue(int voiceIndex) const
{
    /* 每个声部有不同相位偏移 */
    double phaseOffset = static_cast<double>(voiceIndex)
        / static_cast<double>(m_params.voiceCount);
    double phase = m_lfoPhase + phaseOffset;
    phase -= qFloor(phase);

    if (m_params.waveform == LfoWaveform::Sine) {
        return qSin(2.0 * M_PI * phase);
    } else {
        /* 三角波: [-1,1]线性映射 */
        if (phase < 0.25) return 4.0 * phase;
        if (phase < 0.75) return 2.0 - 4.0 * phase;
        return -4.0 + 4.0 * phase;
    }
}

/** @brief 从延迟线读取(线性插值) @param voiceIndex 声部 @param delaySamples 延迟采样数 @return 采样值 */
double ChorusEffect::readDelay(int voiceIndex, double delaySamples) const
{
    if (voiceIndex < 0 || voiceIndex >= m_delayBuffers.size()) return 0.0;

    delaySamples = qBound(1.0, delaySamples, static_cast<double>(m_bufferSize - 1));
    int writeIdx = m_writePos[voiceIndex];
    int delayInt = static_cast<int>(delaySamples);
    double frac = delaySamples - delayInt;

    int readIdx0 = writeIdx - delayInt;
    if (readIdx0 < 0) readIdx0 += m_bufferSize;
    int readIdx1 = readIdx0 - 1;
    if (readIdx1 < 0) readIdx1 += m_bufferSize;

    double s0 = m_delayBuffers[voiceIndex][readIdx0];
    double s1 = m_delayBuffers[voiceIndex][readIdx1];
    return s0 + frac * (s1 - s0);
}

/** @brief 写入延迟线 @param voiceIndex 声部 @param sample 采样值 */
void ChorusEffect::writeDelay(int voiceIndex, double sample)
{
    if (voiceIndex < 0 || voiceIndex >= m_delayBuffers.size()) return;
    m_delayBuffers[voiceIndex][m_writePos[voiceIndex]] = sample;
    m_writePos[voiceIndex] = (m_writePos[voiceIndex] + 1) % m_bufferSize;
}

void ChorusEffect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

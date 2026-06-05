/**
 * @file Reverb.cpp
 * @brief 算法混响引擎实现 — Schroeder+全通+FDN+早晚期混合
 */

#include "utils/dsp22/Reverb.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Reverb::Reverb(QObject* parent)
    : QObject(parent)
    , m_sampleRate(44100.0)
    , m_roomSize(0.5)
    , m_damping(0.5)
    , m_wetLevel(0.3)
    , m_dryLevel(0.7)
    , m_preDelayMs(20.0)
    , m_reverbType(ReverbType::Room)
    , m_preDelayIndex(0)
    , m_earlyLevel(0.4)
    , m_lateLevel(0.6)
    , m_fdnFeedback(0.5)
{
    initFilters();
}

void Reverb::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); initFilters(); }
void Reverb::setReverbType(ReverbType type) { m_reverbType = type; initFilters(); }
void Reverb::setRoomSize(double size) { m_roomSize = qBound(0.0, size, 1.0); initFilters(); }
void Reverb::setDamping(double damping) { m_damping = qBound(0.0, damping, 1.0); }
void Reverb::setWetLevel(double wet) { m_wetLevel = qBound(0.0, wet, 1.0); }
void Reverb::setDryLevel(double dry) { m_dryLevel = qBound(0.0, dry, 1.0); }
void Reverb::setPreDelay(double delayMs) { m_preDelayMs = qMax(0.0, delayMs); initFilters(); }

/**
 * @brief 初始化所有滤波器结构
 */
void Reverb::initFilters()
{
    initSchroeder();
    initFDN();

    /* 预延迟缓冲区 */
    int preDelaySamples = static_cast<int>(m_sampleRate * m_preDelayMs / 1000.0);
    preDelaySamples = qMax(1, preDelaySamples);
    m_preDelayBuffer.assign(preDelaySamples, 0.0);
    m_preDelayIndex = 0;
}

/**
 * @brief 初始化Schroeder梳状+全通滤波器
 */
void Reverb::initSchroeder()
{
    /* 根据混响类型选择延迟时间(ms) */
    double baseScale = 1.0;
    switch (m_reverbType) {
    case ReverbType::Room:    baseScale = 0.6; break;
    case ReverbType::Hall:    baseScale = 1.2; break;
    case ReverbType::Plate:   baseScale = 0.8; break;
    case ReverbType::Spring:  baseScale = 0.5; break;
    }

    double scaledRoom = m_roomSize * baseScale + 0.3;
    double fb = scaledRoom * 0.84;

    /* 梳状滤波器延迟(与采样率成比例) */
    double combDelays[] = {29.7, 37.1, 41.2, 43.7};
    m_combBuffers.resize(kNumCombs);
    m_combIndices.assign(kNumCombs, 0);
    m_combFeedback.resize(kNumCombs);
    m_combDamping.resize(kNumCombs);
    m_combLastFilter.resize(kNumCombs);

    for (int i = 0; i < kNumCombs; ++i) {
        int delaySamples = static_cast<int>(m_sampleRate * combDelays[i] * scaledRoom / 1000.0);
        delaySamples = qMax(4, delaySamples);
        m_combBuffers[i].assign(delaySamples, 0.0);
        m_combFeedback[i] = fb;
        m_combDamping[i] = m_damping;
        m_combLastFilter[i] = 0.0;
    }

    /* 全通滤波器延迟 */
    double allpassDelays[] = {5.0, 1.7};
    m_allpassBuffers.resize(kNumAllpass);
    m_allpassIndices.assign(kNumAllpass, 0);

    for (int i = 0; i < kNumAllpass; ++i) {
        int delaySamples = static_cast<int>(m_sampleRate * allpassDelays[i] / 1000.0);
        delaySamples = qMax(1, delaySamples);
        m_allpassBuffers[i].assign(delaySamples, 0.0);
    }
}

/**
 * @brief 初始化反馈延迟网络
 */
void Reverb::initFDN()
{
    double fdnDelays[] = {31.7, 37.9, 41.3, 44.1};
    m_fdnBuffers.resize(kFDNLines);
    m_fdnIndices.assign(kFDNLines, 0);
    m_fdnFeedback = 0.5 * m_roomSize + 0.2;

    for (int i = 0; i < kFDNLines; ++i) {
        int delaySamples = static_cast<int>(m_sampleRate * fdnDelays[i] / 1000.0);
        delaySamples = qMax(4, delaySamples);
        m_fdnBuffers[i].assign(delaySamples, 0.0);
    }
}

/**
 * @brief 处理完整音频帧
 * @param input 输入采样
 * @return 混响输出
 */
QVector<double> Reverb::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output;
    output.reserve(input.size());

    double peak = 0.0;
    for (double sample : input) {
        double out = processSample(sample);
        output.append(out);
        if (qAbs(out) > peak) peak = qAbs(out);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_stats.totalSamplesProcessed += static_cast<quint64>(input.size());
    ++m_stats.totalFramesRendered;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFramesRendered);

    emit frameProcessed(input.size(), peak);
    return output;
}

/**
 * @brief 处理单个采样点
 * @param input 输入采样值
 * @return 混响输出采样
 */
double Reverb::processSample(double input)
{
    /* 预延迟: 将输入信号延迟一定时间 */
    int preDelayLen = m_preDelayBuffer.size();
    double delayed = m_preDelayBuffer[m_preDelayIndex];
    m_preDelayBuffer[m_preDelayIndex] = input;
    m_preDelayIndex = (m_preDelayIndex + 1) % preDelayLen;

    /* 早期反射: 简单的多次延迟叠加 */
    double early = processEarlyReflections(delayed);

    /* 晚期混响场: Schroeder梳状 + 全通 + FDN */
    double late = processLateField(delayed);

    /* 干湿混合 */
    return m_dryLevel * input + m_wetLevel * (m_earlyLevel * early + m_lateLevel * late);
}

/**
 * @brief 处理梳状滤波器
 * @param input 输入
 * @param filterIdx 滤波器索引
 * @return 输出
 */
double Reverb::processComb(double input, int filterIdx)
{
    auto& buf = m_combBuffers[filterIdx];
    int& idx = m_combIndices[filterIdx];
    int len = buf.size();

    double output = buf[idx];
    /* 一阶低通阻尼 */
    double filtered = output * (1.0 - m_combDamping[filterIdx])
        + m_combLastFilter[filterIdx] * m_combDamping[filterIdx];
    m_combLastFilter[filterIdx] = filtered;

    buf[idx] = input + filtered * m_combFeedback[filterIdx];
    idx = (idx + 1) % len;
    return output;
}

/**
 * @brief 处理全通滤波器
 * @param input 输入
 * @param filterIdx 滤波器索引
 * @return 输出
 */
double Reverb::processAllpass(double input, int filterIdx)
{
    auto& buf = m_allpassBuffers[filterIdx];
    int& idx = m_allpassIndices[filterIdx];
    int len = buf.size();
    double buffered = buf[idx];
    double output = -input + buffered;
    buf[idx] = input + buffered * 0.5;
    idx = (idx + 1) % len;
    return output;
}

/**
 * @brief 早期反射处理
 * @param input 输入信号
 * @return 早期反射信号
 */
double Reverb::Reverb::processEarlyReflections(double input)
{
    /* 简单早期反射模型: 4次延迟叠加 */
    static const double kGains[] = {0.42, 0.38, 0.32, 0.28, 0.22, 0.17};
    static const double kDelayMs[] = {3.5, 7.2, 11.8, 16.4, 21.6, 28.0};

    double sum = 0.0;
    for (int i = 0; i < 6; ++i) {
        int delaySamples = static_cast<int>(m_sampleRate * kDelayMs[i] / 1000.0);
        /* 使用梳状缓冲区的近似延迟读取 */
        int readIdx = (m_combIndices[0] - delaySamples + m_combBuffers[0].size())
            % m_combBuffers[0].size();
        if (readIdx >= 0 && readIdx < m_combBuffers[0].size()) {
            sum += kGains[i] * m_combBuffers[0][readIdx];
        }
    }
    return sum + input * 0.2;
}

/**
 * @brief 晚期混响场处理
 * @param input 输入信号
 * @return 晚期混响信号
 */
double Reverb::processLateField(double input)
{
    /* Schroeder并行梳状 */
    double combSum = 0.0;
    for (int i = 0; i < kNumCombs; ++i) {
        combSum += processComb(input, i);
    }

    /* 串联全通 */
    double apOut = combSum / kNumCombs;
    for (int i = 0; i < kNumAllpass; ++i) {
        apOut = processAllpass(apOut, i);
    }

    /* FDN反馈延迟网络 */
    double fdnSum = 0.0;
    for (int i = 0; i < kFDNLines; ++i) {
        auto& buf = m_fdnBuffers[i];
        int& idx = m_fdnIndices[i];
        int len = buf.size();
        double out = buf[idx];
        buf[idx] = apOut + out * m_fdnFeedback;
        idx = (idx + 1) % len;
        fdnSum += out;
    }

    return apOut + fdnSum / kFDNLines * 0.3;
}

/** @brief 重置统计信息 */
void Reverb::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

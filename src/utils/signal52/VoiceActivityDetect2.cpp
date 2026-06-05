/**
 * @file VoiceActivityDetect2.cpp
 * @brief 语音活动检测(VAD)实现
 *
 * 实现基于能量阈值的语音活动检测算法。通过计算每帧信号的
 * 短时能量并与阈值比较判断是否为语音帧。支持hangover机制
 * （语音结束后的延迟判定）以避免语音尾部被截断。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/signal52/VoiceActivityDetect2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @class VoiceActivityDetect2
 * @brief 基于能量的语音活动检测器
 *
 * 检测流程：
 * 1. 将输入信号按帧分割
 * 2. 计算每帧的短时能量（dB）
 * 3. 与阈值比较得到初始VAD判定
 * 4. 应用hangover机制平滑结果
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
VoiceActivityDetect2::VoiceActivityDetect2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率（Hz）
 */
void VoiceActivityDetect2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置帧大小（样本数）
 * @param samples 每帧的样本数
 */
void VoiceActivityDetect2::setFrameSize(int samples)
{
    m_frameSize = qMax(1, samples);
}

/**
 * @brief 设置能量阈值（dB）
 * @param db 语音/噪声判决阈值
 */
void VoiceActivityDetect2::setThreshold(double db)
{
    m_threshold = db;
}

/**
 * @brief 设置hangover帧数
 * @param frames 语音结束后的延迟帧数，防止语音尾部截断
 */
void VoiceActivityDetect2::setHangover(int frames)
{
    m_hangover = qMax(0, frames);
}

/**
 * @brief 对输入信号执行语音活动检测
 *
 * 1. 将信号按帧分割（帧之间无重叠）
 * 2. 计算每帧的能量（dB）
 * 3. 与阈值比较得到初步判定
 * 4. 应用hangover延迟：检测到语音后，后续hangover帧内
 *    即使能量低于阈值也标记为语音
 *
 * @param signal 输入音频信号
 * @return 每帧的VAD判定结果（1=语音, 0=静音）
 */
QVector<int> VoiceActivityDetect2::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    const int numFrames = (n + m_frameSize - 1) / m_frameSize;
    m_energies.clear();
    m_energies.reserve(numFrames);

    QVector<int> vadFlags(numFrames, 0);

    /* 第一步：计算每帧能量并与阈值比较 */
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_frameSize;
        int end = qMin(start + m_frameSize, n);

        /* 计算帧能量（均方值） */
        double energy = 0.0;
        for (int i = start; i < end; ++i) {
            energy += signal[i] * signal[i];
        }
        energy /= (end - start);

        /* 转换为dB */
        double energyDb = -120.0;
        if (energy > 1e-20) {
            energyDb = 10.0 * qLn(energy) / qLn(10.0);
        }

        m_energies.append(energyDb);

        /* 与阈值比较 */
        vadFlags[f] = (energyDb >= m_threshold) ? 1 : 0;
    }

    /* 第二步：应用hangover机制 */
    int hangoverCount = 0;
    for (int f = 0; f < numFrames; ++f) {
        if (vadFlags[f] == 1) {
            /* 检测到语音：重置hangover计数器 */
            hangoverCount = m_hangover;
        } else if (hangoverCount > 0) {
            /* hangover期间：仍标记为语音 */
            vadFlags[f] = 1;
            hangoverCount--;
        }
    }

    m_stats.totalDetections++;
    m_stats.totalFrames += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDetections > 0)
        ? m_timeSum / m_stats.totalDetections : 0.0;

    /* 检测是否发生语音活动变化 */
    bool hasVoice = false;
    for (int v : vadFlags) {
        if (v == 1) { hasVoice = true; break; }
    }
    emit voiceActivityChanged(hasVoice);

    return vadFlags;
}

/**
 * @brief 重置统计数据
 */
void VoiceActivityDetect2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_energies.clear();
}

/**
 * @brief 使用自适应阈值的语音活动检测
 *
 * 不使用固定阈值，而是基于前noiseFrames帧的平均能量
 * 动态计算阈值。适用于噪声水平未知的场景。
 *
 * @param signal 输入音频信号
 * @param noiseFrames 用于噪声估计的初始帧数
 * @return 每帧的VAD判定结果（1=语音, 0=静音）
 */
QVector<int> VoiceActivityDetect2::detectAdaptive(const QVector<double>& signal, int noiseFrames)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    const int numFrames = (n + m_frameSize - 1) / m_frameSize;
    m_energies.clear();
    m_energies.reserve(numFrames);

    QVector<int> vadFlags(numFrames, 0);

    /* 计算每帧能量 */
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_frameSize;
        int end = qMin(start + m_frameSize, n);

        double energy = 0.0;
        for (int i = start; i < end; ++i) {
            energy += signal[i] * signal[i];
        }
        energy /= (end - start);

        double energyDb = -120.0;
        if (energy > 1e-20) {
            energyDb = 10.0 * qLn(energy) / qLn(10.0);
        }
        m_energies.append(energyDb);
    }

    /* 基于前noiseFrames帧估计噪声能量 */
    double noiseFloor = -120.0;
    int estimateFrames = qMin(noiseFrames, numFrames);
    if (estimateFrames > 0) {
        double sum = 0.0;
        for (int f = 0; f < estimateFrames; ++f) {
            sum += m_energies[f];
        }
        noiseFloor = sum / estimateFrames;
    }

    /* 自适应阈值：噪声底 + 固定偏移 */
    double adaptiveThreshold = noiseFloor + 10.0;  /* 噪声底以上10dB */

    /* 与自适应阈值比较 */
    for (int f = 0; f < numFrames; ++f) {
        vadFlags[f] = (m_energies[f] >= adaptiveThreshold) ? 1 : 0;
    }

    /* 应用hangover机制 */
    int hangoverCount = 0;
    for (int f = 0; f < numFrames; ++f) {
        if (vadFlags[f] == 1) {
            hangoverCount = m_hangover;
        } else if (hangoverCount > 0) {
            vadFlags[f] = 1;
            hangoverCount--;
        }
    }

    m_stats.totalDetections++;
    m_stats.totalFrames += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalDetections > 0)
        ? m_timeSum / m_stats.totalDetections : 0.0;

    bool hasVoice = false;
    for (int v : vadFlags) {
        if (v == 1) { hasVoice = true; break; }
    }
    emit voiceActivityChanged(hasVoice);

    return vadFlags;
}

/**
 * @brief 获取语音帧占总帧数的比例
 * @return 语音活动占比（0.0~1.0），无数据时返回0.0
 */
double VoiceActivityDetect2::voiceRatio() const
{
    if (m_energies.isEmpty()) return 0.0;
    int voiceCount = 0;
    for (double e : m_energies) {
        if (e >= m_threshold) voiceCount++;
    }
    return static_cast<double>(voiceCount) / m_energies.size();
}

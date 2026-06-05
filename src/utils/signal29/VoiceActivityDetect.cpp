/**
 * @file VoiceActivityDetect.cpp
 * @brief 语音活动检测实现 — 能量/ZCR/谱熵/自适应阈值
 */

#include "utils/signal29/VoiceActivityDetect.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
VoiceActivityDetect::VoiceActivityDetect(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void VoiceActivityDetect::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 设置帧长 @param samples 采样数 */
void VoiceActivityDetect::setFrameSize(int samples)
{
    m_frameSize = qMax(16, samples);
}

/** @brief 设置使用的特征组合 @param features 特征位掩码 */
void VoiceActivityDetect::setFeatures(int features)
{
    m_features = features;
}

/** @brief 设置初始能量阈值 @param threshold 阈值 */
void VoiceActivityDetect::setEnergyThreshold(double threshold)
{
    m_energyThreshold = qMax(0.0, threshold);
}

/** @brief 设置自适应因子 @param alpha 平滑因子(0~1) */
void VoiceActivityDetect::setAdaptiveAlpha(double alpha)
{
    m_alpha = qBound(0.0, alpha, 1.0);
}

/** @brief 检测单帧 @param frame 音频帧 @return VAD结果 */
VoiceActivityDetect::VadResult VoiceActivityDetect::detect(
    const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    VadResult result;

    /* 计算各特征 */
    if (m_features & static_cast<int>(Feature::Energy)) {
        result.energy = computeEnergy(frame);
    }
    if (m_features & static_cast<int>(Feature::ZCR)) {
        result.zcr = computeZCR(frame);
    }
    if (m_features & static_cast<int>(Feature::SpectralEntropy)) {
        result.spectralEntropy = computeSpectralEntropy(frame);
    }

    /* 多特征融合判定 */
    int voteCount = 0;
    int totalFeatures = 0;
    double confidenceSum = 0.0;

    if (m_features & static_cast<int>(Feature::Energy)) {
        ++totalFeatures;
        double adaptiveThresh = m_bgEnergy * (1.0 + m_energyThreshold / m_bgEnergy);
        adaptiveThresh = qMax(adaptiveThresh, m_energyThreshold);
        if (result.energy > adaptiveThresh) {
            ++voteCount;
            confidenceSum += qMin(1.0, result.energy / adaptiveThresh - 1.0);
        }
    }

    if (m_features & static_cast<int>(Feature::ZCR)) {
        ++totalFeatures;
        /* 语音段ZCR通常低于纯噪声 */
        if (result.zcr < m_zcrThreshold) {
            ++voteCount;
            confidenceSum += qMin(1.0, (m_zcrThreshold - result.zcr) / m_zcrThreshold);
        }
    }

    if (m_features & static_cast<int>(Feature::SpectralEntropy)) {
        ++totalFeatures;
        /* 语音段谱熵低于噪声 */
        double adaptiveEntropyThresh = m_bgEntropy * 1.2;
        if (result.spectralEntropy < adaptiveEntropyThresh) {
            ++voteCount;
            confidenceSum += qMin(1.0,
                (adaptiveEntropyThresh - result.spectralEntropy) / adaptiveEntropyThresh);
        }
    }

    /* 多数投票: 超过半数特征认为是语音 */
    result.isVoice = (totalFeatures > 0 && voteCount > totalFeatures / 2);
    result.confidence = (totalFeatures > 0)
        ? qMin(1.0, confidenceSum / totalFeatures) : 0.0;

    /* 更新自适应阈值 */
    updateThresholds(result);

    /* 统计 */
    double elapsed = timer.elapsed();
    m_stats.totalFramesProcessed++;
    if (result.isVoice) m_stats.totalVoiceFrames++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFramesProcessed);
    m_energySum += result.energy;
    m_stats.avgEnergy = m_energySum
        / static_cast<double>(m_stats.totalFramesProcessed);
    m_zcrSum += result.zcr;
    m_stats.avgZcr = m_zcrSum
        / static_cast<double>(m_stats.totalFramesProcessed);

    /* 发射信号 */
    if (result.isVoice) {
        emit voiceDetected(m_frameIndex, result.energy);
    } else {
        emit silenceDetected(m_frameIndex);
    }
    ++m_frameIndex;

    return result;
}

/** @brief 批量检测 @param audio 完整音频 @return 逐帧结果列表 */
QVector<VoiceActivityDetect::VadResult> VoiceActivityDetect::detectBatch(
    const QVector<double>& audio)
{
    QVector<VadResult> results;
    int total = audio.size();
    int hop = m_frameSize / 2; /* 50%重叠 */
    if (hop < 1) hop = 1;

    for (int offset = 0; offset + m_frameSize <= total; offset += hop) {
        QVector<double> frame(m_frameSize);
        for (int i = 0; i < m_frameSize; ++i) {
            frame[i] = audio[offset + i];
        }
        results.append(detect(frame));
    }

    return results;
}

/** @brief 计算短时能量 @param frame 音频帧 @return 能量 */
double VoiceActivityDetect::computeEnergy(
    const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;

    double sum = 0.0;
    for (double s : frame) {
        sum += s * s;
    }
    return sum / static_cast<double>(frame.size());
}

/** @brief 计算过零率 @param frame 音频帧 @return 过零率(0~1) */
double VoiceActivityDetect::computeZCR(
    const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;

    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0)) {
            ++crossings;
        }
    }
    return static_cast<double>(crossings) / static_cast<double>(frame.size() - 1);
}

/** @brief 计算谱熵 @param frame 音频帧 @return 谱熵(0~1) */
double VoiceActivityDetect::computeSpectralEntropy(
    const QVector<double>& frame) const
{
    int n = frame.size();
    if (n < 4) return 0.0;

    /* 简易DFT计算功率谱 */
    int halfN = n / 2;
    QVector<double> power(halfN, 0.0);

    for (int k = 0; k < halfN; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * i * k / n;
            re += frame[i] * qCos(angle);
            im += frame[i] * qSin(angle);
        }
        power[k] = (re * re + im * im) / n;
    }

    /* 归一化为概率分布 */
    double totalPower = 0.0;
    for (double p : power) totalPower += p;
    if (totalPower <= 0.0) return 1.0;

    /* 计算Shannon熵 */
    double entropy = 0.0;
    for (double p : power) {
        double prob = p / totalPower;
        if (prob > 0.0) {
            entropy -= prob * qLn(prob) / qLn(2.0);
        }
    }

    /* 归一化到0~1 */
    double maxEntropy = qLn(static_cast<double>(halfN)) / qLn(2.0);
    return (maxEntropy > 0) ? entropy / maxEntropy : 0.0;
}

/** @brief 更新自适应阈值 @param result 当前帧结果 */
void VoiceActivityDetect::updateThresholds(const VadResult& result)
{
    if (!result.isVoice) {
        /* 静音帧更新背景估计 */
        m_bgEnergy = m_alpha * m_bgEnergy
            + (1.0 - m_alpha) * result.energy;
        m_bgEntropy = m_alpha * m_bgEntropy
            + (1.0 - m_alpha) * result.spectralEntropy;
    }
}

/** @brief 重置统计 */
void VoiceActivityDetect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_energySum = 0.0;
    m_zcrSum = 0.0;
    m_frameIndex = 0;
    m_bgEnergy = 0.001;
    m_bgEntropy = 0.5;
}

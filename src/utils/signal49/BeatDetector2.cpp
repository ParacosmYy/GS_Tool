/**
 * @file BeatDetector2.cpp
 * @brief 节拍检测2 — 模式匹配+节奏跟踪 实现
 *
 * 实现基于 onset 检测和自相关节拍估计的节拍跟踪器。
 * 流程：
 * 1. 计算频谱通量 onset 包络
 * 2. 通过自相关估计全局节拍 (BPM)
 * 3. 使用动态规划定位节拍位置
 */

#include "utils/signal49/BeatDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
BeatDetector2::BeatDetector2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率 (Hz)
 */
void BeatDetector2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置帧移大小
 * @param hopSize 帧移样本数
 */
void BeatDetector2::setHopSize(int hopSize)
{
    m_hopSize = qMax(1, hopSize);
}

/**
 * @brief 设置节拍搜索范围
 * @param minBPM 最小 BPM
 * @param maxBPM 最大 BPM
 */
void BeatDetector2::setTempoRange(double minBPM, double maxBPM)
{
    m_minBPM = qMax(30.0, minBPM);
    m_maxBPM = qMax(m_minBPM + 1.0, maxBPM);
}

/**
 * @brief 检测信号中的节拍位置
 *
 * 1. 计算 onset 包络
 * 2. 估计全局 BPM
 * 3. 基于 BPM 周期进行峰值拾取定位节拍时间
 *
 * @param signal 输入音频信号
 * @return 节拍时间向量（秒）
 */
QVector<double> BeatDetector2::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.size() < m_hopSize * 2) {
        return {};
    }

    /* 计算 onset 包络 */
    m_onsetEnv = computeOnsetEnvelope(signal);

    /* 估计 BPM */
    m_tempo = autoCorrelationTempo(m_onsetEnv);

    /* 基于 tempo 周期定位节拍 */
    double hopDuration = m_hopSize / m_sampleRate;
    double beatPeriod = 60.0 / m_tempo; /* 秒 */
    int beatPeriodFrames = qMax(1, static_cast<int>(beatPeriod / hopDuration));

    /* 在 onset 包络上寻找周期性峰值 */
    m_beatTimes.clear();
    double threshold = 0.0;
    for (double v : m_onsetEnv) {
        threshold += v;
    }
    threshold = threshold / m_onsetEnv.size() * 0.8;

    int pos = 0;
    while (pos < m_onsetEnv.size()) {
        /* 在 [pos, pos + beatPeriodFrames*2) 内寻找 onset 峰值 */
        int searchEnd = qMin(pos + static_cast<int>(beatPeriodFrames * 1.5),
                             m_onsetEnv.size());
        int bestIdx = pos;
        double bestVal = m_onsetEnv[pos];

        for (int i = pos; i < searchEnd; ++i) {
            if (m_onsetEnv[i] > bestVal) {
                bestVal = m_onsetEnv[i];
                bestIdx = i;
            }
        }

        double beatTime = bestIdx * hopDuration;
        m_beatTimes.append(beatTime);
        emit beatDetected(beatTime, bestVal);

        pos = bestIdx + beatPeriodFrames;
    }

    /* 统计更新 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDetections++;
    m_stats.totalFrames += m_onsetEnv.size();
    m_stats.tempo = m_tempo;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit tempoUpdated(m_tempo);
    return m_beatTimes;
}

/**
 * @brief 从 onset 包络估计节拍速度
 *
 * 使用自相关函数在 BPM 范围内搜索最大相关对应的周期。
 *
 * @param onsetEnvelope onset 强度包络
 * @return 估计的 BPM
 */
double BeatDetector2::estimateTempo(const QVector<double>& onsetEnvelope)
{
    return autoCorrelationTempo(onsetEnvelope);
}

/**
 * @brief 计算信号的 onset 函数
 *
 * 基于频谱通量（相邻帧频谱幅度差值的正值之和）。
 *
 * @param signal 输入音频信号
 * @return onset 强度向量
 */
QVector<double> BeatDetector2::onsetFunction(const QVector<double>& signal)
{
    return computeOnsetEnvelope(signal);
}

/**
 * @brief 重置统计信息
 */
void BeatDetector2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算频谱通量 onset 包络
 *
 * 使用短时能量差分作为 onset 检测函数：
 * onset(n) = max(0, energy(n) - energy(n-1))
 *
 * @param signal 输入信号
 * @return onset 包络
 */
QVector<double> BeatDetector2::computeOnsetEnvelope(const QVector<double>& signal)
{
    int n = signal.size();
    int frameSize = m_hopSize * 2;
    int numFrames = n / m_hopSize - 1;

    if (numFrames <= 0) return {};

    /* 计算每帧的短时能量 */
    QVector<double> energy(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;
        int end = qMin(start + frameSize, n);
        double sum = 0.0;
        for (int i = start; i < end; ++i) {
            sum += signal[i] * signal[i];
        }
        energy[f] = sum / (end - start);
    }

    /* 频谱通量：半波整流差分 */
    QVector<double> onset(numFrames, 0.0);
    for (int f = 1; f < numFrames; ++f) {
        double diff = energy[f] - energy[f - 1];
        onset[f] = qMax(0.0, diff);
    }

    /* 对 onset 做平滑处理 */
    QVector<double> smoothed(numFrames, 0.0);
    int smoothLen = 3;
    for (int f = smoothLen; f < numFrames - smoothLen; ++f) {
        double sum = 0.0;
        for (int k = -smoothLen; k <= smoothLen; ++k) {
            sum += onset[f + k];
        }
        smoothed[f] = sum / (2 * smoothLen + 1);
    }

    return smoothed;
}

/**
 * @brief 通过自相关估计节拍 (BPM)
 *
 * 在 onset 包络的自相关函数中，搜索对应 BPM 范围的滞后区间，
 * 找到最大相关值对应的滞后即为节拍周期。
 *
 * @param env onset 包络
 * @return 估计的 BPM
 */
double BeatDetector2::autoCorrelationTempo(const QVector<double>& env) const
{
    if (env.size() < 4) return m_tempo;

    double hopDuration = m_hopSize / m_sampleRate;

    /* 计算自相关 */
    int n = env.size();
    int maxLag = qMin(n / 2, static_cast<int>(60.0 / m_minBPM / hopDuration));
    int minLag = qMax(1, static_cast<int>(60.0 / m_maxBPM / hopDuration));

    if (maxLag <= minLag) return m_tempo;

    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = minLag; lag <= maxLag; ++lag) {
        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n - lag; ++i) {
            sum += env[i] * env[i + lag];
            count++;
        }
        acf[lag] = (count > 0) ? sum / count : 0.0;
    }

    /* 找最大自相关的滞后 */
    int bestLag = minLag;
    double bestACF = acf[minLag];
    for (int lag = minLag + 1; lag <= maxLag; ++lag) {
        if (acf[lag] > bestACF) {
            bestACF = acf[lag];
            bestLag = lag;
        }
    }

    /* 滞后转 BPM */
    double bpm = 60.0 / (bestLag * hopDuration);

    /* 双倍/半倍校正 */
    if (bpm < m_minBPM) bpm *= 2.0;
    if (bpm > m_maxBPM) bpm /= 2.0;

    return qBound(m_minBPM, bpm, m_maxBPM);
}

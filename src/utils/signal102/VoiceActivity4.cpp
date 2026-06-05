#include "VoiceActivity4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file VoiceActivity4.cpp
 * @brief 语音活动检测器实现
 *
 * 基于短时能量和过零率双特征检测语音活动:
 * - 能量特征: 语音段能量显著高于噪声
 * - 过零率: 语音的清音段过零率较高
 * 综合两个特征做出最终判决。
 */

/**
 * @brief 构造函数，初始化默认检测参数
 * @param parent 父QObject对象指针
 */
VoiceActivity4::VoiceActivity4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置能量阈值
 * @param thresholdDb 语音/噪声判定的能量阈值(dB)
 */
void VoiceActivity4::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置分析窗口大小
 * @param size 每帧的采样点数
 */
void VoiceActivity4::setWindowSize(int size)
{
    m_windowSize = qMax(2, size);
}

/**
 * @brief 检测语音活动
 *
 * 对输入信号分帧处理，每帧计算:
 * 1. 短时能量(dB)
 * 2. 短时过零率
 * 综合两个特征判定当前帧是否为语音段。
 *
 * @param samples 输入音频采样数据
 * @return 各帧的语音活动标记(true=有语音)
 */
QVector<bool> VoiceActivity4::detect(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const int frameCount = (N + m_windowSize - 1) / m_windowSize;
    QVector<bool> result(frameCount, false);

    const double thresholdLin = std::pow(10.0, m_threshold / 20.0);

    for (int frame = 0; frame < frameCount; ++frame) {
        const int start = frame * m_windowSize;
        const int end = qMin(start + m_windowSize, N);

        // 计算短时能量
        double energy = 0.0;
        for (int i = start; i < end; ++i) {
            energy += samples[i] * samples[i];
        }
        energy = std::sqrt(energy / (end - start));

        // 计算过零率
        int zeroCrossings = 0;
        for (int i = start + 1; i < end; ++i) {
            if ((samples[i] >= 0.0 && samples[i - 1] < 0.0) ||
                (samples[i] < 0.0 && samples[i - 1] >= 0.0)) {
                zeroCrossings++;
            }
        }
        const double zcr = static_cast<double>(zeroCrossings) / (end - start);

        // 综合判定
        if (energy > thresholdLin) {
            result[frame] = true;
        } else if (energy > thresholdLin * 0.5 && zcr > 0.1) {
            // 低能量但高过零率可能是清音
            result[frame] = true;
        }
    }

    // 平滑处理: 消除孤立帧
    for (int i = 1; i < frameCount - 1; ++i) {
        if (result[i] && !result[i - 1] && !result[i + 1]) {
            result[i] = false; // 去除孤立语音帧
        }
    }

    // 统计语音帧数
    int voiceFrames = 0;
    for (bool v : result) {
        if (v) voiceFrames++;
    }

    // 更新统计信息
    m_stats.totalFrames += frameCount;
    m_stats.voiceFrames += voiceFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalFrames / frameCount);

    emit detected(voiceFrames > 0);
    return result;
}

/**
 * @brief 重置所有统计信息
 */
void VoiceActivity4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

#include "VoiceActivity3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化语音活动检测器
 * @param parent 父对象指针
 */
VoiceActivity3::VoiceActivity3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置能量检测阈值(dB)
 * @param thresholdDb 语音/静音判定阈值
 */
void VoiceActivity3::setThreshold(double thresholdDb)
{
    m_threshold = thresholdDb;
}

/**
 * @brief 设置分析帧大小
 * @param size 帧大小(采样点数)
 */
void VoiceActivity3::setFrameSize(int size)
{
    m_frameSize = qMax(64, size);
}

/**
 * @brief 对输入信号帧检测语音活动
 *
 * 综合使用短时能量和过零率两个指标进行VAD判定：
 * 1. 计算帧能量(dB)，与阈值比较
 * 2. 计算过零率，高过零率可能为噪声
 * 3. 两个指标加权综合判定
 *
 * @param frame 输入信号帧
 * @return true表示检测到语音活动，false表示静音/噪声
 */
bool VoiceActivity3::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 2) {
        m_timeSum += timer.elapsed();
        m_stats.totalDetections++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;
        emit detected(false);
        return false;
    }

    const int N = frame.size();

    /* 计算帧能量(RMS) */
    double energySum = 0.0;
    for (int i = 0; i < N; ++i) {
        energySum += frame[i] * frame[i];
    }
    double rms = std::sqrt(energySum / N);
    double energyDb = 20.0 * std::log10(qMax(1e-10, rms));

    /* 计算过零率 */
    int zeroCrossings = 0;
    for (int i = 1; i < N; ++i) {
        if ((frame[i] >= 0) != (frame[i - 1] >= 0)) {
            zeroCrossings++;
        }
    }
    double zcr = static_cast<double>(zeroCrossings) / (N - 1);

    /* 综合判定 */
    bool isVoice = false;

    /* 能量判定：高于阈值为潜在语音 */
    bool energyActive = (energyDb > m_threshold);

    /* 过零率判定：过高过零率(>0.5)倾向为噪声 */
    bool zcrActive = (zcr < 0.5);

    /* 综合决策 */
    if (energyActive && zcrActive) {
        isVoice = true;
    } else if (energyActive && !zcrActive) {
        /* 高能量但高过零率：可能是高频噪声 */
        isVoice = (energyDb > m_threshold + 10.0);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalDetections++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;
    emit detected(isVoice);
    return isVoice;
}

/**
 * @brief 重置统计数据
 */
void VoiceActivity3::resetStatistics()
{
    m_stats.totalDetections = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

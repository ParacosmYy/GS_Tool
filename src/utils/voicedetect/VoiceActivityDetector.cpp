/**
 * @file VoiceActivityDetector.cpp
 * @brief 语音活动检测器实现
 */

#include "utils/voicedetect/VoiceActivityDetector.h"

#include <QElapsedTimer>
#include <QtMath>

VoiceActivityDetector::VoiceActivityDetector(QObject* parent)
    : QObject(parent), m_energyThreshold(0.05),
      m_zcrThreshold(0.15), m_frameSize(256),
      m_hopSize(128), m_timeSum(0.0) {}

void VoiceActivityDetector::setEnergyThreshold(double threshold)
{
    m_energyThreshold = qBound(0.0, threshold, 1.0);
}

void VoiceActivityDetector::setZcrThreshold(double threshold)
{
    m_zcrThreshold = qBound(0.0, threshold, 1.0);
}

void VoiceActivityDetector::setFrameParams(int frameSize, int hopSize)
{
    m_frameSize = qMax(32, frameSize);
    m_hopSize = qMax(1, hopSize);
}

VoiceActivityDetector::VadResult VoiceActivityDetector::detect(
    const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    VadResult result;
    int nFrames = (data.size() - m_frameSize) / m_hopSize + 1;
    if (nFrames < 1) nFrames = 1;
    result.totalFrames = nFrames;

    /* 第一遍: 计算每帧能量和过零率 */
    double maxEnergy = 0.0;
    for (int i = 0; i < nFrames; ++i) {
        int start = i * m_hopSize;
        double energy = computeFrameEnergy(data, start, m_frameSize);
        double zcr = computeFrameZcr(data, start, m_frameSize);
        result.frameEnergy.append(energy);
        result.frameZcr.append(zcr);
        if (energy > maxEnergy) maxEnergy = energy;
    }

    /* 归一化能量 */
    if (maxEnergy > 0) {
        for (auto& e : result.frameEnergy) e /= maxEnergy;
    }

    /* 第二遍: 双门限检测 */
    for (int i = 0; i < nFrames; ++i) {
        bool voiced = (result.frameEnergy[i] >= m_energyThreshold) &&
                      (result.frameZcr[i] <= m_zcrThreshold);
        if (voiced) {
            result.voicedFrames.append(i);
        } else {
            result.silentFrames.append(i);
        }
    }

    m_stats.totalDetections++;
    m_stats.totalVoicedFrames += result.voicedFrames.size();
    m_stats.totalSilentFrames += result.silentFrames.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalDetections, 1ULL);

    emit detectionCompleted(result.voicedFrames.size(), nFrames);
    return result;
}

double VoiceActivityDetector::computeFrameEnergy(
    const QVector<double>& data, int start, int length) const
{
    double sum = 0.0;
    int end = qMin(start + length, data.size());
    for (int i = start; i < end; ++i) {
        sum += data[i] * data[i];
    }
    return sum / (end - start);
}

double VoiceActivityDetector::computeFrameZcr(
    const QVector<double>& data, int start, int length) const
{
    int crossings = 0;
    int end = qMin(start + length, data.size());
    for (int i = start + 1; i < end; ++i) {
        if ((data[i - 1] >= 0.0 && data[i] < 0.0) ||
            (data[i - 1] < 0.0 && data[i] >= 0.0)) {
            ++crossings;
        }
    }
    return static_cast<double>(crossings) / (end - start - 1);
}

void VoiceActivityDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

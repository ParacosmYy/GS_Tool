/**
 * @file ShortTimeEnergy.cpp
 * @brief 短时能量实现 — 语音端点检测
 */

#include "utils/shorttime/ShortTimeEnergy.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ShortTimeEnergy::ShortTimeEnergy(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算短时能量
 *  @param signal 输入信号
 *  @param frameSize 帧长(样本数)
 *  @param hopSize 帧移(样本数)
 *  @return 每帧能量值 */
QVector<double> ShortTimeEnergy::compute(const QVector<double>& signal,
                                         int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    frameSize = qMax(1, frameSize);
    hopSize = qMax(1, hopSize);

    int numFrames = (n - frameSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    QVector<double> energy(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        double e = 0.0;
        for (int i = 0; i < frameSize && start + i < n; ++i) {
            e += signal[start + i] * signal[start + i];
        }
        energy[f] = e / static_cast<double>(frameSize);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(numFrames);
    return energy;
}

/** @brief 基于能量阈值的语音段检测
 *  @param signal 输入信号
 *  @param threshold 能量阈值(0~1, 相对最大能量)
 *  @return 语音段列表 */
QVector<ShortTimeEnergy::VoiceSegment> ShortTimeEnergy::detectVoice(
    const QVector<double>& signal, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    int frameSize = 256;
    int hopSize = 128;
    QVector<double> energy = compute(signal, frameSize, hopSize);

    if (energy.isEmpty()) { return {}; }

    /* 归一化到[0,1] */
    double maxE = *std::max_element(energy.begin(), energy.end());
    double absThreshold = maxE * qBound(0.0, threshold, 1.0);

    QVector<bool> voiced(energy.size(), false);
    for (int i = 0; i < energy.size(); ++i) {
        voiced[i] = energy[i] >= absThreshold;
    }

    /* 提取连续有声段 */
    QVector<VoiceSegment> segments;
    int i = 0;
    while (i < voiced.size()) {
        if (voiced[i]) {
            VoiceSegment seg;
            seg.startFrame = i;
            double eSum = 0.0;
            while (i < voiced.size() && voiced[i]) {
                eSum += energy[i];
                ++i;
            }
            seg.endFrame = i - 1;
            int dur = seg.endFrame - seg.startFrame + 1;
            seg.energy = eSum / static_cast<double>(dur);
            segments.push_back(seg);
        } else {
            ++i;
        }
    }

    double elapsed = static_cast<double>(timer.elapsed());
    /* compute()已更新统计 */

    emit voiceDetected(segments.size());
    return segments;
}

/** @brief 重置统计 */
void ShortTimeEnergy::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

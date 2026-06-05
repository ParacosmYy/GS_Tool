#include "BeatDetector3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化节拍检测器
 * @param parent 父QObject对象指针
 */
BeatDetector3::BeatDetector3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 从音频信号中检测节拍位置
 *
 * 基于能量包络的峰值检测算法：
 * 1. 将信号分帧计算短时能量
 * 2. 对能量包络进行平滑处理
 * 3. 检测能量峰值作为节拍候选
 * 4. 使用自适应阈值过滤弱峰值
 *
 * @param samples 输入音频采样数据
 * @param sampleRate 采样率(Hz)
 * @return 节拍位置列表（采样点索引）
 */
QVector<int> BeatDetector3::detectBeats(const QVector<double>& samples, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = samples.size();
    if (n == 0) return {};

    /// 帧参数设置
    const int frameSize = static_cast<int>(sampleRate * 0.02);  ///< 20ms帧长
    const int hopSize = frameSize / 2;                           ///< 50%重叠
    const int numFrames = (n - frameSize) / hopSize + 1;

    if (numFrames <= 0) return {};

    /// 步骤1：计算短时能量包络
    QVector<double> energy(numFrames);
    for (int f = 0; f < numFrames; ++f) {
        double sum = 0.0;
        int start = f * hopSize;
        for (int i = 0; i < frameSize && start + i < n; ++i) {
            sum += samples[start + i] * samples[start + i];
        }
        energy[f] = sum / frameSize;
    }

    /// 步骤2：一阶差分（只保留上升沿）
    QVector<double> diff(numFrames - 1);
    for (int i = 0; i < numFrames - 1; ++i) {
        diff[i] = qMax(0.0, energy[i + 1] - energy[i]);
    }

    /// 步骤3：计算自适应阈值（局部均值+标准差）
    const int windowSize = qMax(4, numFrames / 16);
    QVector<double> threshold(diff.size(), 0.0);
    for (int i = 0; i < static_cast<int>(diff.size()); ++i) {
        int start = qMax(0, i - windowSize / 2);
        int end = qMin(static_cast<int>(diff.size()), i + windowSize / 2);
        double mean = 0.0;
        for (int j = start; j < end; ++j) mean += diff[j];
        mean /= (end - start);
        threshold[i] = mean * 1.5;  ///< 阈值为局部均值的1.5倍
    }

    /// 步骤4：峰值检测
    QVector<int> beatPositions;
    int minBeatInterval = static_cast<int>(sampleRate * 0.15 / hopSize);  ///< 最小节拍间隔150ms
    int lastBeat = -minBeatInterval;

    for (int i = 1; i < static_cast<int>(diff.size()) - 1; ++i) {
        if (diff[i] > threshold[i] && diff[i] > diff[i - 1] && diff[i] >= diff[i + 1]) {
            if (i - lastBeat >= minBeatInterval) {
                int samplePos = (i + 1) * hopSize;
                beatPositions.append(samplePos);
                double confidence = diff[i] / qMax(1e-10, threshold[i]);
                emit beatDetected(samplePos, qMin(confidence, 1.0));
                lastBeat = i;
            }
        }
    }

    /// 更新统计信息
    m_stats.totalBeatsDetected += beatPositions.size();
    m_stats.totalBuffersAnalyzed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuffersAnalyzed;

    return beatPositions;
}

/**
 * @brief 估计音频信号的BPM(每分钟节拍数)
 *
 * 基于检测到的节拍位置，计算相邻节拍间隔的中位数，
 * 转换为BPM值。使用中位数而非均值以增强鲁棒性。
 *
 * @param samples 输入音频采样数据
 * @param sampleRate 采样率(Hz)
 * @return 估计的BPM值，无节拍时返回0
 */
double BeatDetector3::estimateBPM(const QVector<double>& samples, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> beats = detectBeats(samples, sampleRate);
    if (beats.size() < 2) {
        m_lastBPM = 0.0;
        return 0.0;
    }

    /// 计算相邻节拍间隔
    QVector<double> intervals;
    intervals.reserve(beats.size() - 1);
    for (int i = 1; i < beats.size(); ++i) {
        intervals.append(beats[i] - beats[i - 1]);
    }

    /// 取中位数间隔
    std::sort(intervals.begin(), intervals.end());
    double medianInterval = intervals[intervals.size() / 2];

    /// 转换为BPM
    double intervalSeconds = medianInterval / sampleRate;
    m_lastBPM = (intervalSeconds > 0.0) ? (60.0 / intervalSeconds) : 0.0;

    /// 限制合理BPM范围
    m_lastBPM = qBound(30.0, m_lastBPM, 300.0);
    return m_lastBPM;
}

/**
 * @brief 获取当前统计数据
 * @return 包含节拍数、缓冲区数和平均耗时的Stats结构
 */
BeatDetector3::Stats BeatDetector3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void BeatDetector3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_lastBPM = 0.0;
}

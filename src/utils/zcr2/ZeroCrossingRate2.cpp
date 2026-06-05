/**
 * @file ZeroCrossingRate2.cpp
 * @brief 过零率V2实现 — 带阈值+分帧
 */

#include "utils/zcr2/ZeroCrossingRate2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ZeroCrossingRate2::ZeroCrossingRate2(QObject* parent)
    : QObject(parent)
    , m_voiceThreshold(0.1)
    , m_timeSum(0.0)
{
}

/** @brief 计算分帧过零率
 *  @param signal 输入信号
 *  @param frameSize 帧长(样本数)
 *  @param threshold 过零阈值(幅度容差)
 *  @return 每帧过零率 */
QVector<double> ZeroCrossingRate2::compute(const QVector<double>& signal,
                                           int frameSize, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    frameSize = qMax(1, frameSize);
    int hopSize = frameSize / 2;
    if (hopSize < 1) hopSize = 1;

    int numFrames = (n - frameSize) / hopSize + 1;
    if (numFrames < 1) numFrames = 1;

    QVector<double> zcr(numFrames, 0.0);
    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;
        int count = 0;
        for (int i = 1; i < frameSize && start + i < n; ++i) {
            double prev = signal[start + i - 1];
            double curr = signal[start + i];
            /* 带阈值过零: 符号变化且幅度超过阈值 */
            if ((prev > threshold && curr < -threshold) ||
                (prev < -threshold && curr > threshold)) {
                ++count;
            }
        }
        zcr[f] = static_cast<double>(count) / static_cast<double>(frameSize - 1);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalComputations;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(numFrames);
    return zcr;
}

/** @brief 基于过零率的语音段检测
 *  @param signal 输入信号
 *  @return 有声标志列表(true=有声) */
QVector<bool> ZeroCrossingRate2::detectVoice(const QVector<double>& signal)
{
    QVector<double> zcr = compute(signal, 256, 0.0);
    QVector<bool> voiced(zcr.size());

    for (int i = 0; i < zcr.size(); ++i) {
        /* 高过零率通常对应无声/噪声段 */
        voiced[i] = zcr[i] < m_voiceThreshold;
    }

    return voiced;
}

/** @brief 计算全信号过零率
 *  @param signal 输入信号
 *  @param threshold 过零阈值
 *  @return 过零率 */
double ZeroCrossingRate2::computeGlobal(const QVector<double>& signal,
                                        double threshold) const
{
    int n = signal.size();
    if (n < 2) return 0.0;

    int count = 0;
    for (int i = 1; i < n; ++i) {
        double prev = signal[i - 1];
        double curr = signal[i];
        if ((prev > threshold && curr < -threshold) ||
            (prev < -threshold && curr > threshold)) {
            ++count;
        }
    }

    return static_cast<double>(count) / static_cast<double>(n - 1);
}

/** @brief 设置语音检测过零率阈值 @param threshold 阈值 */
void ZeroCrossingRate2::setVoiceThreshold(double threshold)
{
    m_voiceThreshold = qBound(0.0, threshold, 1.0);
}

/** @brief 重置统计 */
void ZeroCrossingRate2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

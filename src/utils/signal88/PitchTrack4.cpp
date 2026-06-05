#include "PitchTrack4.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class PitchTrack4
 * @brief 基音追踪器实现
 *
 * 基于自相关函数(ACF)的基频(F0)追踪算法。
 * 自相关函数衡量信号与其延迟版本的相关性，
 * 在基频周期处出现峰值。通过寻找第一个显著峰值确定基频。
 *
 * 支持单帧和批量追踪模式。批量模式使用滑动窗口，
 * 可选YIN算法的绝对差值阈值改进精度。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
PitchTrack4::PitchTrack4(QObject* parent)
    : QObject(parent)
    , m_lastPitch(0.0)
{
}

/**
 * @brief 追踪单帧基频
 *
 * 使用自相关法检测基频:
 * 1. 计算信号的自相关函数 r(τ) = Σ x(t)x(t+τ)
 * 2. 归一化自相关函数
 * 3. 在合理基频范围内(50~800Hz)寻找第一个显著峰值
 * 4. 使用抛物线插值提高频率分辨率
 *
 * @param frame 输入帧(建议20~60ms窗口)
 * @param sampleRate 采样率(Hz)
 * @return 检测到的基频(Hz)，0表示无声帧
 */
double PitchTrack4::trackFrame(const QVector<double>& frame, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    int N = frame.size();
    if (N < 64 || sampleRate <= 0.0) {
        m_timeSum += timer.elapsed();
        return 0.0;
    }

    /* 计算信号能量，过低则判定为无声 */
    double energy = 0.0;
    for (int i = 0; i < N; ++i) {
        energy += frame[i] * frame[i];
    }
    energy /= N;

    if (energy < 1e-8) {
        m_stats.totalFramesTracked++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesTracked);
        return 0.0;
    }

    /* 计算自相关函数 */
    int minLag = qMax(1, static_cast<int>(sampleRate / 800.0)); /* 最高800Hz */
    int maxLag = qMin(N / 2, static_cast<int>(sampleRate / 50.0)); /* 最低50Hz */

    QVector<double> acf(maxLag + 1, 0.0);
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i) {
            sum += frame[i] * frame[i + lag];
        }
        acf[lag] = sum / (N - lag);
    }

    /* 归一化 */
    if (acf[0] > 1e-15) {
        for (int lag = 0; lag <= maxLag; ++lag) {
            acf[lag] /= acf[0];
        }
    }

    /* 在[minLag, maxLag]范围内寻找第一个峰值 */
    int bestLag = 0;
    double bestVal = 0.0;
    bool inPeak = false;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (acf[lag] > 0.0 && !inPeak) {
            inPeak = true;
        }
        if (inPeak && acf[lag] > bestVal) {
            bestVal = acf[lag];
            bestLag = lag;
        }
        if (inPeak && lag > minLag && acf[lag] < bestVal * 0.8) {
            break; /* 峰值已过 */
        }
    }

    double freqHz = 0.0;
    double confidence = 0.0;

    if (bestLag > 0 && bestVal > 0.3) {
        /* 抛物线插值提高精度 */
        if (bestLag > 0 && bestLag < maxLag) {
            double y0 = acf[bestLag - 1];
            double y1 = acf[bestLag];
            double y2 = acf[bestLag + 1];
            double delta = (y0 - y2) / (2.0 * (y0 - 2.0 * y1 + y2 + 1e-15));
            double refinedLag = bestLag + delta;
            freqHz = sampleRate / qMax(1.0, refinedLag);
        } else {
            freqHz = sampleRate / bestLag;
        }
        confidence = bestVal;
        m_stats.totalVoicedFrames++;
        m_lastPitch = freqHz;
    }

    m_stats.totalFramesTracked++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesTracked);

    emit pitchDetected(freqHz, confidence);

    return freqHz;
}

/**
 * @brief 批量追踪基频序列
 *
 * 使用滑动窗口逐帧提取基频，输出基频随时间的变化曲线。
 *
 * @param samples 完整音频采样序列
 * @param sampleRate 采样率(Hz)
 * @param frameSize 帧大小(采样点)
 * @param hopSize 帧移(采样点)
 * @return 基频序列(Hz)，无声帧为0
 */
QVector<double> PitchTrack4::trackSequence(const QVector<double>& samples, double sampleRate,
                                            int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> pitchCurve;

    if (samples.size() < frameSize || hopSize <= 0) {
        m_timeSum += timer.elapsed();
        return pitchCurve;
    }

    for (int start = 0; start + frameSize <= samples.size(); start += hopSize) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = samples[start + i];
        }
        double pitch = trackFrame(frame, sampleRate);
        pitchCurve.append(pitch);
    }

    m_timeSum += timer.elapsed();

    return pitchCurve;
}

/**
 * @brief 重置所有统计数据
 *
 * 将帧追踪计数、有声帧计数和计时归零。
 */
void PitchTrack4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_lastPitch = 0.0;
}

/**
 * @file PitchShift.cpp
 * @brief 变调处理器实现
 */

#include "PitchShift.h"
#include <QElapsedTimer>
#include <cmath>

PitchShift::PitchShift(int fftSize, int hopSize, QObject* parent)
    : QObject(parent)
    , m_fftSize(qMax(256, fftSize))
    , m_hopSize(qMax(64, hopSize))
    , m_timeSum(0.0)
{
}

QVector<double> PitchShift::process(const QVector<double>& input, double semitones)
{
    double ratio = std::pow(2.0, semitones / 12.0);
    return processByRatio(input, ratio);
}

QVector<double> PitchShift::processByRatio(const QVector<double>& input, double ratio)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {};

    ratio = qBound(0.25, ratio, 4.0);
    int halfFFT = m_fftSize / 2;

    /* 重采样方法: 简化的时域拉伸+重采样 */
    int outputLen = static_cast<int>(N / ratio);
    if (outputLen <= 0) outputLen = 1;

    /* 步骤1: 时域拉伸(通过线性插值改变采样位置) */
    QVector<double> stretched(N);
    for (int i = 0; i < N; ++i) {
        double srcIdx = static_cast<double>(i) * ratio;
        int idx0 = static_cast<int>(srcIdx);
        int idx1 = idx0 + 1;
        double frac = srcIdx - idx0;

        if (idx0 >= 0 && idx0 < N) {
            double v0 = input[idx0];
            double v1 = (idx1 < N) ? input[idx1] : v0;
            stretched[i] = v0 + frac * (v1 - v0);
        } else if (idx0 >= N) {
            stretched[i] = 0.0;
        }
    }

    /* 步骤2: 加Hanning窗的OLA(Overlap-Add)平滑 */
    QVector<double> output(outputLen, 0.0);
    QVector<double> window(m_fftSize);

    for (int i = 0; i < m_fftSize; ++i)
        window[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / m_fftSize));

    QVector<double> winSum(outputLen, 0.0);

    for (int pos = 0; pos + m_fftSize <= N; pos += m_hopSize) {
        for (int j = 0; j < m_fftSize; ++j) {
            int outIdx = pos + j;
            if (outIdx < outputLen) {
                output[outIdx] += stretched[pos + j] * window[j];
                winSum[outIdx] += window[j] * window[j];
            }
        }
        m_stats.totalFrames++;
    }

    /* 归一化窗函数增益 */
    for (int i = 0; i < outputLen; ++i) {
        if (winSum[i] > 1e-10)
            output[i] /= winSum[i];
    }

    m_stats.totalProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(N, outputLen, 12.0 * std::log2(ratio));
    return output;
}

void PitchShift::setParameters(int fftSize, int hopSize)
{
    m_fftSize = qMax(256, fftSize);
    m_hopSize = qMax(64, hopSize);
}

PitchShift::Stats PitchShift::stats() const { return m_stats; }

void PitchShift::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

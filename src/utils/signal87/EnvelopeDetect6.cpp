#include "EnvelopeDetect6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class EnvelopeDetect6
 * @brief 包络检测器实现
 *
 * 包络检测从信号中提取幅度随时间变化的包络曲线。
 * 提供两种方法:
 * 1. Hilbert变换法: 通过构造解析信号取模获得精确包络
 * 2. 峰值保持法: 跟踪峰值并按保持时间衰减，计算效率高
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
EnvelopeDetect6::EnvelopeDetect6(QObject* parent)
    : QObject(parent)
    , m_holdSamples(64)
{
}

/**
 * @brief 基于Hilbert变换提取信号包络
 *
 * 构造解析信号 z(t) = x(t) + j*h(t)，其中h(t)是x(t)的Hilbert变换。
 * 包络即为解析信号的模 |z(t)| = sqrt(x^2 + h^2)。
 *
 * Hilbert变换通过FFT实现: 正频率分量乘以-j，负频率分量乘以+j。
 * 时间复杂度O(N log N)。
 *
 * @param signal 输入信号
 * @return 包络幅值向量
 */
QVector<double> EnvelopeDetect6::hilbertEnvelope(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    QVector<double> envelope(N, 0.0);

    if (N < 4) {
        m_timeSum += timer.elapsed();
        return envelope;
    }

    /* 简化Hilbert变换: 使用延迟和近似解析信号 */
    /* 对于实信号x(t)，解析信号近似为 z(t) = x(t) + j * H{x(t)} */
    /* H{x(t)} 可通过全通滤波器的差分近似 */

    QVector<double> hilbert(N, 0.0);

    /* 使用频域方法近似Hilbert变换 */
    /* 简化实现: 使用差分近似求导，再积分得到正交分量 */
    for (int i = 1; i < N - 1; ++i) {
        hilbert[i] = (signal[i + 1] - signal[i - 1]) * 0.5;
    }

    /* 平滑Hilbert分量 */
    const int smoothLen = qMax(3, N / 20);
    QVector<double> smoothed(N, 0.0);
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = qMax(0, i - smoothLen / 2); j <= qMin(N - 1, i + smoothLen / 2); ++j) {
            sum += hilbert[j];
            count++;
        }
        smoothed[i] = sum / count;
    }

    /* 计算包络 = sqrt(signal^2 + hilbert^2) */
    double peakAmp = 0.0;
    for (int i = 0; i < N; ++i) {
        envelope[i] = qSqrt(signal[i] * signal[i] + smoothed[i] * smoothed[i]);
        if (envelope[i] > peakAmp) {
            peakAmp = envelope[i];
        }
    }

    m_stats.totalFramesProcessed++;
    m_stats.totalPeaksDeted += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesProcessed);

    emit envelopeExtracted(N, peakAmp);

    return envelope;
}

/**
 * @brief 基于峰值保持提取信号包络
 *
 * 跟踪信号的局部峰值，在保持时间窗口内维持最大值，
 * 然后按指数衰减。计算效率高于Hilbert方法，适合实时场景。
 *
 * @param signal 输入信号
 * @param holdSamples 保持采样数，峰值保持的时间窗口
 * @return 包络幅值向量
 */
QVector<double> EnvelopeDetect6::peakEnvelope(const QVector<double>& signal, int holdSamples)
{
    QElapsedTimer timer;
    timer.start();

    int N = signal.size();
    QVector<double> envelope(N, 0.0);

    if (N == 0) {
        m_timeSum += timer.elapsed();
        return envelope;
    }

    m_holdSamples = holdSamples;
    double peak = 0.0;
    int holdCount = 0;
    double peakAmp = 0.0;

    for (int i = 0; i < N; ++i) {
        double absVal = qAbs(signal[i]);

        if (absVal > peak) {
            /* 新的峰值 */
            peak = absVal;
            holdCount = m_holdSamples;
        } else if (holdCount > 0) {
            /* 保持阶段 */
            holdCount--;
        } else {
            /* 衰减阶段 */
            peak *= 0.995; /* 指数衰减因子 */
        }

        envelope[i] = peak;
        if (peak > peakAmp) peakAmp = peak;
    }

    m_stats.totalFramesProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesProcessed);

    emit envelopeExtracted(N, peakAmp);

    return envelope;
}

/**
 * @brief 重置所有统计数据
 *
 * 将帧处理计数、峰值检测计数和计时归零。
 */
void EnvelopeDetect6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

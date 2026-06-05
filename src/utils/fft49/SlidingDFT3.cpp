/**
 * @file SlidingDFT3.cpp
 * @brief 滑动DFT3 — 稳定化递推+频率跟踪 实现
 *
 * 实现 Goertzel 风格的滑动 DFT，每个新采样以 O(N) 更新
 * 全部频率 bin。包含幅度衰减稳定化机制防止递推累积误差。
 * 支持单频跟踪和批量频率分析。
 */

#include "utils/fft49/SlidingDFT3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
SlidingDFT3::SlidingDFT3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置 FFT 大小和采样率
 *
 * 重新计算所有 bin 的旋转因子系数，
 * 并清空内部状态缓冲区。
 *
 * @param fftSize FFT 大小（应为 2 的幂）
 * @param sampleRate 采样率（Hz）
 */
void SlidingDFT3::setParameters(int fftSize, double sampleRate)
{
    m_fftSize = qMax(4, fftSize);
    m_sampleRate = qMax(1.0, sampleRate);
    m_initialized = false;
    computeCoefficients();
}

/**
 * @brief 计算各 bin 的旋转因子系数
 *
 * 对第 k 个 bin，cosCoeff[k] = cos(2*pi*k/N), sinCoeff[k] = -sin(2*pi*k/N)。
 * 同时初始化实部、虚部和循环缓冲区。
 */
void SlidingDFT3::computeCoefficients()
{
    const int N = m_fftSize;
    m_cosCoeff.resize(N);
    m_sinCoeff.resize(N);
    m_realPart.assign(N, 0.0);
    m_imagPart.assign(N, 0.0);
    m_circularBuffer.assign(N, 0.0);
    m_bufferIdx = 0;

    for (int k = 0; k < N; ++k) {
        double angle = 2.0 * M_PI * k / N;
        m_cosCoeff[k] = qCos(angle);
        m_sinCoeff[k] = -qSin(angle);
    }

    m_stats.numBins = N;
    m_initialized = true;
}

/**
 * @brief 推入一个新的采样值，更新所有频率 bin
 *
 * 使用稳定化递推公式：
 *   X_k[n] = (X_k[n-1] - x[n-N]) * e^{-j2pi*k/N} + x[n]
 * 每次更新后对幅度做微小衰减以防止数值发散。
 *
 * @param sample 新的采样值
 */
void SlidingDFT3::pushSample(double sample)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) {
        computeCoefficients();
    }

    const int N = m_fftSize;

    /* 取出最旧的采样 */
    double oldest = m_circularBuffer[m_bufferIdx];

    /* 更新循环缓冲区 */
    m_circularBuffer[m_bufferIdx] = sample;
    m_bufferIdx = (m_bufferIdx + 1) % N;

    /* 稳定化衰减因子（极接近1.0，防止数值累积误差） */
    const double stabilization = 1.0 - 1e-10;

    /* 递推更新每个频率 bin */
    for (int k = 0; k < N; ++k) {
        /* 去除最旧采样的贡献并加入新采样 */
        double realOld = m_realPart[k] - oldest;
        double imagOld = m_imagPart[k];

        /* 旋转：乘以 e^{-j2pi*k/N} */
        m_realPart[k] = (realOld * m_cosCoeff[k] - imagOld * m_sinCoeff[k] + sample) * stabilization;
        m_imagPart[k] = (realOld * m_sinCoeff[k] + imagOld * m_cosCoeff[k]) * stabilization;
    }

    /* 统计更新 */
    m_stats.totalUpdates++;
    m_stats.totalSamplesProcessed++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    if (m_stats.totalUpdates > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;
    }
}

/**
 * @brief 获取所有频率 bin 的幅度谱
 * @return 幅度向量，长度为 fftSize
 */
QVector<double> SlidingDFT3::magnitudes() const
{
    const int N = m_fftSize;
    QVector<double> mag(N);
    for (int k = 0; k < N; ++k) {
        mag[k] = qSqrt(m_realPart[k] * m_realPart[k] + m_imagPart[k] * m_imagPart[k]);
    }
    return mag;
}

/**
 * @brief 获取所有频率 bin 的相位谱
 * @return 相位向量（弧度），长度为 fftSize
 */
QVector<double> SlidingDFT3::phases() const
{
    const int N = m_fftSize;
    QVector<double> phase(N);
    for (int k = 0; k < N; ++k) {
        phase[k] = qAtan2(m_imagPart[k], m_realPart[k]);
    }
    return phase;
}

/**
 * @brief 获取指定频率处的幅度
 * @param freq 目标频率（Hz）
 * @return 该频率 bin 的幅度值
 */
double SlidingDFT3::magnitudeAt(double freq) const
{
    int bin = qRound(freq * m_fftSize / m_sampleRate);
    bin = qBound(0, bin, m_fftSize - 1);
    return qSqrt(m_realPart[bin] * m_realPart[bin] + m_imagPart[bin] * m_imagPart[bin]);
}

/**
 * @brief 获取指定频率处的相位
 * @param freq 目标频率（Hz）
 * @return 该频率 bin 的相位值（弧度）
 */
double SlidingDFT3::phaseAt(double freq) const
{
    int bin = qRound(freq * m_fftSize / m_sampleRate);
    bin = qBound(0, bin, m_fftSize - 1);
    return qAtan2(m_imagPart[bin], m_realPart[bin]);
}

/**
 * @brief 跟踪指定频率在整个信号中的幅度变化
 *
 * 逐步推入信号采样，记录目标频率 bin 的幅度变化轨迹。
 *
 * @param freq 目标频率（Hz）
 * @param signal 输入信号
 * @return 幅度轨迹向量
 */
QVector<double> SlidingDFT3::trackFrequency(double freq, const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int bin = qRound(freq * m_fftSize / m_sampleRate);
    bin = qBound(0, bin, m_fftSize - 1);

    /* 重置状态以获得干净跟踪 */
    m_realPart.assign(m_fftSize, 0.0);
    m_imagPart.assign(m_fftSize, 0.0);
    m_circularBuffer.assign(m_fftSize, 0.0);
    m_bufferIdx = 0;

    const int N = m_fftSize;
    const double stabilization = 1.0 - 1e-10;
    QVector<double> track;
    track.reserve(signal.size());

    for (int i = 0; i < signal.size(); ++i) {
        double sample = signal[i];
        double oldest = m_circularBuffer[m_bufferIdx];
        m_circularBuffer[m_bufferIdx] = sample;
        m_bufferIdx = (m_bufferIdx + 1) % N;

        double realOld = m_realPart[bin] - oldest;
        double imagOld = m_imagPart[bin];
        m_realPart[bin] = (realOld * m_cosCoeff[bin] - imagOld * m_sinCoeff[bin] + sample) * stabilization;
        m_imagPart[bin] = (realOld * m_sinCoeff[bin] + imagOld * m_cosCoeff[bin]) * stabilization;

        double mag = qSqrt(m_realPart[bin] * m_realPart[bin] + m_imagPart[bin] * m_imagPart[bin]);
        track.append(mag);

        m_stats.totalSamplesProcessed++;
    }

    /* 统计更新 */
    m_stats.totalUpdates += signal.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit frequencyTracked(freq, track.isEmpty() ? 0.0 : track.last());
    return track;
}

/**
 * @brief 重置所有统计数据
 */
void SlidingDFT3::resetStatistics()
{
    m_stats = Stats{};
    m_stats.numBins = m_fftSize;
    m_timeSum = 0.0;
}

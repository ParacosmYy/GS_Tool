#include "GoertzelAlgorithm6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Goertzel算法计算器
 * @param parent 父对象指针
 */
GoertzelAlgorithm6::GoertzelAlgorithm6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置目标检测频率(Hz)
 * @param freq 目标频率，算法将计算该频率的DFT值
 */
void GoertzelAlgorithm6::setTargetFreq(double freq)
{
    m_targetFreq = qMax(0.0, freq);
}

/**
 * @brief 设置采样率(Hz)
 * @param rate 采样率
 */
void GoertzelAlgorithm6::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 对采样数据执行Goertzel计算
 *
 * Goertzel算法通过二阶IIR滤波器结构高效计算
 * 指定频率k上的DFT值，复杂度O(N)，无需完整FFT。
 *
 * 滤波器递推: s[n] = x[n] + 2*cos(2*pi*k/N)*s[n-1] - s[n-2]
 * 输出: |X[k]|^2 = s[N-1]^2 + s[N-2]^2 - 2*cos(2*pi*k/N)*s[N-1]*s[N-2]
 *
 * @param samples 输入采样数据
 */
void GoertzelAlgorithm6::compute(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    if (samples.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalComputed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        emit computationCompleted(0.0);
        return;
    }

    const int N = samples.size();

    /* 计算归一化频率 */
    double k = m_targetFreq * N / m_sampleRate;
    double coeff = 2.0 * std::cos(2.0 * M_PI * k / N);

    /* Goertzel递推 */
    double s0 = 0.0; /* s[n] */
    double s1 = 0.0; /* s[n-1] */
    double s2 = 0.0; /* s[n-2] */

    for (int n = 0; n < N; ++n) {
        s0 = samples[n] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算幅值 */
    double magnitudeSquared = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    double magnitude = std::sqrt(qMax(0.0, magnitudeSquared)) / N;

    /* 计算功率(可用于DTMF判定) */
    double power = magnitudeSquared / (N * N);

    m_timeSum += timer.elapsed();
    m_stats.totalComputed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
    emit computationCompleted(magnitude);
}

/**
 * @brief 对多个目标频率执行批量Goertzel计算
 * @param samples 输入采样
 * @param targetFreqs 目标频率列表
 * @return 每个频率对应的幅值
 */
QVector<double> GoertzelAlgorithm6::computeMulti(
    const QVector<double>& samples,
    const QVector<double>& targetFreqs) const
{
    QVector<double> magnitudes;
    if (samples.isEmpty() || targetFreqs.isEmpty()) return magnitudes;
    const int N = samples.size();
    for (double freq : targetFreqs) {
        double k = freq * N / m_sampleRate;
        double coeff = 2.0 * std::cos(2.0 * M_PI * k / N);
        double s0 = 0.0, s1 = 0.0, s2 = 0.0;
        for (int n = 0; n < N; ++n) {
            s0 = samples[n] + coeff * s1 - s2;
            s2 = s1; s1 = s0;
        }
        double mag2 = s1 * s1 + s2 * s2 - coeff * s1 * s2;
        magnitudes.append(std::sqrt(qMax(0.0, mag2)) / N);
    }
    return magnitudes;
}

/**
 * @brief 计算目标频率的相位
 * @param samples 输入采样
 * @return 相位角(弧度)
 */
double GoertzelAlgorithm6::phase(const QVector<double>& samples) const
{
    if (samples.isEmpty()) return 0.0;
    const int N = samples.size();
    double k = m_targetFreq * N / m_sampleRate;
    double coeff = 2.0 * std::cos(2.0 * M_PI * k / N);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int n = 0; n < N; ++n) {
        s0 = samples[n] + coeff * s1 - s2;
        s2 = s1; s1 = s0;
    }
    double re = s1 - s2 * std::cos(2.0 * M_PI * k / N);
    double im = s2 * std::sin(2.0 * M_PI * k / N);
    return std::atan2(im, re);
}

/**
 * @brief 重置统计数据
 */
void GoertzelAlgorithm6::resetStatistics()
{
    m_stats.totalComputed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

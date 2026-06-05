/**
 * @file EnvelopeDetect3.cpp
 * @brief 包络检测3实现 — 多方法包络+能量跟踪
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal44/EnvelopeDetect3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
EnvelopeDetect3::EnvelopeDetect3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("EnvelopeDetect3"));
}

/**
 * @brief 设置检测方法
 *
 * 支持四种包络检测方法：
 * - Hilbert: 通过解析信号（Hilbert变换）提取包络，最准确
 * - Rectification: 全波整流+低通滤波，简单快速
 * - Squaring: 平方+低通+开方，适合音频RMS
 * - PeakHold: 峰值保持+指数衰减，适合瞬态分析
 *
 * @param method 检测方法
 */
void EnvelopeDetect3::setMethod(Method method)
{
    m_method = method;
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率（Hz）
 */
void EnvelopeDetect3::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置平滑时间常数
 *
 * 时间常数控制低通滤波器的截止频率。
 * 较大的时间常数产生更平滑的包络。
 *
 * @param timeConstant 时间常数（秒）
 */
void EnvelopeDetect3::setSmoothing(double timeConstant)
{
    m_smoothing = qMax(0.001, timeConstant);
}

/**
 * @brief 执行包络检测
 *
 * 根据当前设置的方法计算信号包络，
 * 并更新峰值和RMS包络统计。
 *
 * @param signal 输入信号
 * @return 包络信号
 */
QVector<double> EnvelopeDetect3::detect(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> env;

    switch (m_method) {
    case Hilbert:
        env = hilbertEnvelope(signal);
        break;
    case Rectification:
        env = rectificationEnvelope(signal);
        break;
    case Squaring:
        env = squaringEnvelope(signal);
        break;
    case PeakHold:
        env = peakHoldEnvelope(signal);
        break;
    }

    /* 更新统计 */
    m_peakEnvelope = 0.0;
    m_rmsEnvelope = 0.0;
    for (double v : env) {
        m_peakEnvelope = qMax(m_peakEnvelope, v);
        m_rmsEnvelope += v * v;
    }
    if (!env.isEmpty()) {
        m_rmsEnvelope = qSqrt(m_rmsEnvelope / env.size());
    }

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += signal.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(signal.size(), m_peakEnvelope, m_rmsEnvelope);
    return env;
}

/**
 * @brief Hilbert变换包络检测
 *
 * 通过简化的FIR滤波器近似Hilbert变换，
 * 构造解析信号后取模作为包络。
 *
 * @param signal 输入信号
 * @return Hilbert包络
 */
QVector<double> EnvelopeDetect3::hilbertEnvelope(const QVector<double>& signal)
{
    int N = signal.size();
    if (N == 0) return {};

    /* 简化Hilbert变换：使用FIR滤波器近似 */
    /* 对每个样本，用邻近样本加权和估计90度相移 */
    int halfLen = qMin(32, N / 4);
    QVector<double> hilbert(N, 0.0);

    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 1; k <= halfLen; ++k) {
            double coeff = (k % 2 == 0) ? 0.0 : 2.0 / (M_PI * k);
            if (n + k < N) sum += signal[n + k] * coeff;
            if (n - k >= 0) sum -= signal[n - k] * coeff;
        }
        hilbert[n] = sum;
    }

    /* 解析信号模值 */
    QVector<double> env(N);
    for (int i = 0; i < N; ++i) {
        env[i] = qSqrt(signal[i] * signal[i] + hilbert[i] * hilbert[i]);
    }

    /* 低通平滑 */
    double alpha = 1.0 / (1.0 + m_sampleRate * m_smoothing);
    for (int i = 1; i < N; ++i) {
        env[i] = alpha * env[i] + (1.0 - alpha) * env[i - 1];
    }

    return env;
}

/**
 * @brief 全波整流包络检测
 *
 * 对信号取绝对值后通过一阶IIR低通滤波器平滑。
 * 最简单快速的方法。
 *
 * @param signal 输入信号
 * @return 整流包络
 */
QVector<double> EnvelopeDetect3::rectificationEnvelope(const QVector<double>& signal)
{
    int N = signal.size();
    if (N == 0) return {};

    QVector<double> env(N);
    double alpha = 1.0 / (1.0 + m_sampleRate * m_smoothing);

    env[0] = qFabs(signal[0]);
    for (int i = 1; i < N; ++i) {
        double rectified = qFabs(signal[i]);
        env[i] = alpha * rectified + (1.0 - alpha) * env[i - 1];
    }

    return env;
}

/**
 * @brief 平方律包络检测
 *
 * 对信号平方后低通滤波，再取平方根。
 * 等价于RMS检测，对高斯噪声最优。
 *
 * @param signal 输入信号
 * @return 平方律包络
 */
QVector<double> EnvelopeDetect3::squaringEnvelope(const QVector<double>& signal)
{
    int N = signal.size();
    if (N == 0) return {};

    QVector<double> env(N);
    double alpha = 1.0 / (1.0 + m_sampleRate * m_smoothing);

    env[0] = signal[0] * signal[0];
    for (int i = 1; i < N; ++i) {
        double squared = signal[i] * signal[i];
        env[i] = alpha * squared + (1.0 - alpha) * env[i - 1];
    }

    /* 取平方根 */
    for (int i = 0; i < N; ++i) {
        env[i] = qSqrt(qMax(0.0, env[i]));
    }

    return env;
}

/**
 * @brief 峰值保持包络检测
 *
 * 跟踪信号峰值，以指数速率衰减。
 * 适合检测瞬态事件和脉冲。
 *
 * @param signal 输入信号
 * @return 峰值保持包络
 */
QVector<double> EnvelopeDetect3::peakHoldEnvelope(const QVector<double>& signal)
{
    int N = signal.size();
    if (N == 0) return {};

    QVector<double> env(N);
    double decay = qExp(-1.0 / (m_sampleRate * m_smoothing));

    env[0] = qFabs(signal[0]);
    for (int i = 1; i < N; ++i) {
        double absVal = qFabs(signal[i]);
        env[i] = qMax(absVal, env[i - 1] * decay);
    }

    return env;
}

/**
 * @brief 重置所有统计数据
 */
void EnvelopeDetect3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_peakEnvelope = 0.0;
    m_rmsEnvelope = 0.0;
}

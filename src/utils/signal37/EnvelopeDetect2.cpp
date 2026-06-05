/**
 * @file EnvelopeDetect2.cpp
 * @brief 多方法包络检测实现 — Hilbert/峰值/RMS/对数RMS包络
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持四种包络检测方法：
 * - Hilbert：基于 Hilbert 变换的解析信号包络
 * - PeakDetect：峰值保持包络
 * - RMS：均方根包络（滑动窗口）
 * - LogRMS：对数 RMS 包络（dB 量级）
 */

#include "utils/signal37/EnvelopeDetect2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
EnvelopeDetect2::EnvelopeDetect2(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("EnvelopeDetect2"));
}

/**
 * @brief 重置统计信息
 */
void EnvelopeDetect2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设置包络检测方法
 * @param method 检测方法枚举
 */
void EnvelopeDetect2::setMethod(Method method)
{
    m_method = method;
}

/**
 * @brief 设置 attack 时间常数
 *
 * attack 控制包络上升速度，影响检测器对瞬时峰值的跟踪能力。
 *
 * @param ms 毫秒数
 */
void EnvelopeDetect2::setAttackTime(double ms)
{
    m_attack = qMax(0.01, ms);
}

/**
 * @brief 设置 release 时间常数
 *
 * release 控制包络下降速度，影响检测器的衰减特性。
 *
 * @param ms 毫秒数
 */
void EnvelopeDetect2::setReleaseTime(double ms)
{
    m_release = qMax(0.01, ms);
}

/**
 * @brief 设置采样率
 * @param rate 采样率 (Hz)
 */
void EnvelopeDetect2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 基-2 原地 FFT 实现
 * @param re 实部数组
 * @param im 虚部数组
 * @param n FFT 点数
 * @param inverse 是否逆变换
 */
static void fftImpl(QVector<double> &re, QVector<double> &im, int n, bool inverse)
{
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }

    for (int len = 2; len <= n; len <<= 1) {
        const double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        const double wRe = qCos(angle);
        const double wIm = qSin(angle);
        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                const double tRe = curRe * re[i+j+len/2] - curIm * im[i+j+len/2];
                const double tIm = curRe * im[i+j+len/2] + curIm * re[i+j+len/2];
                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
                const double nRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = nRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }
    }
}

/**
 * @brief Hilbert 变换包络检测
 *
 * 通过 FFT 构造解析信号，取模得到瞬时幅度包络。
 *
 * @param input 输入信号
 * @return 包络值序列
 */
static QVector<double> hilbertEnvelope(const QVector<double> &input)
{
    const int n = input.size();
    int fftSize = 1;
    while (fftSize < n) fftSize <<= 1;

    QVector<double> re(fftSize, 0.0);
    QVector<double> im(fftSize, 0.0);
    for (int i = 0; i < n; ++i) re[i] = input[i];

    fftImpl(re, im, fftSize, false);

    // 构造解析信号：正频率 *2，负频率置零
    for (int i = 1; i < fftSize / 2; ++i) {
        re[i] *= 2.0; im[i] *= 2.0;
    }
    for (int i = fftSize / 2 + 1; i < fftSize; ++i) {
        re[i] = 0.0; im[i] = 0.0;
    }

    fftImpl(re, im, fftSize, true);

    QVector<double> envelope(n);
    for (int i = 0; i < n; ++i) {
        envelope[i] = qSqrt(re[i] * re[i] + im[i] * im[i]);
    }
    return envelope;
}

/**
 * @brief 峰值保持包络检测
 *
 * 使用 attack/release 时间常数进行峰值跟踪：
 * - 信号上升时快速跟随
 * - 信号下降时按 release 衰减
 *
 * @param input 输入信号
 * @param attack 攻击时间 (ms)
 * @param release 释放时间 (ms)
 * @param sampleRate 采样率
 * @return 包络值序列
 */
static QVector<double> peakEnvelope(const QVector<double> &input,
                                     double attack, double release, double sampleRate)
{
    const int n = input.size();
    QVector<double> envelope(n, 0.0);
    double env = 0.0;

    const double attackCoeff = qExp(-1.0 / (attack * sampleRate * 0.001));
    const double releaseCoeff = qExp(-1.0 / (release * sampleRate * 0.001));

    for (int i = 0; i < n; ++i) {
        const double absVal = qFabs(input[i]);
        if (absVal > env) {
            env = attackCoeff * env + (1.0 - attackCoeff) * absVal;
        } else {
            env = releaseCoeff * env + (1.0 - releaseCoeff) * absVal;
        }
        envelope[i] = env;
    }
    return envelope;
}

/**
 * @brief RMS 包络检测
 *
 * 滑动窗口内计算均方根值作为包络。
 *
 * @param input 输入信号
 * @param windowSize RMS 窗口大小（样本数）
 * @return 包络值序列
 */
static QVector<double> rmsEnvelope(const QVector<double> &input, int windowSize)
{
    const int n = input.size();
    QVector<double> envelope(n, 0.0);
    windowSize = qMax(1, windowSize);

    double sumSq = 0.0;

    for (int i = 0; i < n; ++i) {
        sumSq += input[i] * input[i];
        if (i >= windowSize) {
            sumSq -= input[i - windowSize] * input[i - windowSize];
        }
        const int count = qMin(i + 1, windowSize);
        envelope[i] = qSqrt(sumSq / count);
    }
    return envelope;
}

/**
 * @brief 对数 RMS 包络检测
 *
 * 先计算 RMS 包络，再转换为 dB 量级。
 * 使用 20 * log10(rms + epsilon) 避免 log(0)。
 *
 * @param input 输入信号
 * @param windowSize RMS 窗口大小
 * @return 包络值序列（dB）
 */
static QVector<double> logRmsEnvelope(const QVector<double> &input, int windowSize)
{
    QVector<double> rms = rmsEnvelope(input, windowSize);
    for (double &val : rms) {
        val = 20.0 * qLn(val + 1e-12) / qLn(10.0);
    }
    return rms;
}

/**
 * @brief 批量包络检测
 *
 * 根据当前设置的方法对输入信号执行包络检测。
 * 处理完成后发射 detectionComplete 信号。
 *
 * @param input 输入信号
 * @return 包络值序列
 */
QVector<double> EnvelopeDetect2::detect(const QVector<double> &input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;

    switch (m_method) {
    case Hilbert:
        result = hilbertEnvelope(input);
        break;
    case PeakDetect:
        result = peakEnvelope(input, m_attack, m_release, m_sampleRate);
        break;
    case RMS: {
        const int windowSize = qMax(1, static_cast<int>(m_release * m_sampleRate * 0.001));
        result = rmsEnvelope(input, windowSize);
        break;
    }
    case LogRMS: {
        const int windowSize = qMax(1, static_cast<int>(m_release * m_sampleRate * 0.001));
        result = logRmsEnvelope(input, windowSize);
        break;
    }
    }

    // 更新统计
    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += input.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionComplete(input.size());
    return result;
}

/**
 * @brief 单样本包络检测（流式处理）
 *
 * 使用 attack/release 系数进行单点包络跟踪。
 * 适用于实时流式处理场景。
 *
 * @param sample 输入样本
 * @return 当前包络值
 */
double EnvelopeDetect2::detectOne(double sample)
{
    const double absVal = qFabs(sample);
    const double attackCoeff = qExp(-1.0 / (m_attack * m_sampleRate * 0.001));
    const double releaseCoeff = qExp(-1.0 / (m_release * m_sampleRate * 0.001));

    if (absVal > m_envelope) {
        m_envelope = attackCoeff * m_envelope + (1.0 - attackCoeff) * absVal;
    } else {
        m_envelope = releaseCoeff * m_envelope + (1.0 - releaseCoeff) * absVal;
    }

    m_envelope = qMax(0.0, m_envelope);
    return m_envelope;
}

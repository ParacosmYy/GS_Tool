/**
 * @file Resampler.cpp
 * @brief 多相重采样器实现
 */

#include "utils/signal11/Resampler.h"

#include <QtMath>
#include <QtGlobal>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
Resampler::Resampler(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置重采样参数
 * 计算L/M比率，设计多相抗混叠滤波器
 */
bool Resampler::configure(double inputRate, double outputRate,
                           int filterLength,
                           double stopbandAttenuation)
{
    if (inputRate <= 0.0 || outputRate <= 0.0) return false;

    m_inputRate = inputRate;
    m_outputRate = outputRate;

    /* 计算有理数比率 L/M = outputRate/inputRate (最简分数) */
    double ratio = outputRate / inputRate;
    double numer = ratio;
    double denom = 1.0;

    /* 精度限制: 最大65536 */
    for (int i = 0; i < 16; ++i) {
        double frac = numer - std::floor(numer);
        if (frac < 1e-10) break;
        numer *= 10.0;
        denom *= 10.0;
    }

    int L = static_cast<int>(std::round(numer));
    int M = static_cast<int>(std::round(denom));
    int g = gcd(L, M);
    m_L = L / g;
    m_M = M / g;

    /* 限制因子范围 */
    if (m_L > 65536) m_L = 65536;
    if (m_M > 65536) m_M = 65536;
    if (m_L <= 0 || m_M <= 0) return false;

    /* 设计多相滤波器 */
    designPolyphaseFilter(filterLength, stopbandAttenuation);

    /* 初始化延迟线 */
    m_delaySize = m_tapsPerPhase;
    m_delayLine.assign(m_delaySize, 0.0);
    m_delayPos = 0;
    m_phaseIndex = 0;
    m_sampleIndex = 0;
    m_configured = true;

    return true;
}

/**
 * @brief 处理输入样本
 * 多相分解: 每个输出样本只计算一个相位的FIR滤波
 */
QVector<double> Resampler::process(const QVector<double>& input)
{
    if (!m_configured || input.isEmpty()) return {};

    m_timer.start();
    QVector<double> output;

    for (int i = 0; i < input.size(); ++i) {
        /* 写入延迟线 */
        m_delayLine[m_delayPos] = input[i];
        m_delayPos = (m_delayPos + 1) % m_delaySize;

        /* 计算输出样本 */
        while (m_sampleIndex < m_L) {
            int phase = m_sampleIndex % m_L;
            const QVector<double>& taps = m_polyPhase[phase];

            double sum = 0.0;
            for (int t = 0; t < m_tapsPerPhase; ++t) {
                int idx = (m_delayPos - 1 - t + m_delaySize) % m_delaySize;
                sum += taps[t] * m_delayLine[idx];
            }

            output.append(sum);
            m_sampleIndex += m_M;
        }

        m_sampleIndex -= m_L;
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalResamples;
    m_stats.totalInputSamples += input.size();
    m_stats.totalOutputSamples += output.size();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalResamples);

    emit processCompleted(input.size(), output.size());
    return output;
}

/**
 * @brief 冲刷残余样本
 */
QVector<double> Resampler::flush()
{
    if (!m_configured) return {};

    m_timer.start();
    QVector<double> output;

    /* 用零填充延迟线剩余部分 */
    int remaining = m_tapsPerPhase;
    for (int i = 0; i < remaining; ++i) {
        m_delayLine[m_delayPos] = 0.0;
        m_delayPos = (m_delayPos + 1) % m_delaySize;

        while (m_sampleIndex < m_L) {
            int phase = m_sampleIndex % m_L;
            const QVector<double>& taps = m_polyPhase[phase];

            double sum = 0.0;
            for (int t = 0; t < m_tapsPerPhase; ++t) {
                int idx = (m_delayPos - 1 - t + m_delaySize) % m_delaySize;
                sum += taps[t] * m_delayLine[idx];
            }

            output.append(sum);
            m_sampleIndex += m_M;
        }

        m_sampleIndex -= m_L;
    }

    m_timeSum += m_timer.elapsed();
    m_stats.totalOutputSamples += output.size();

    return output;
}

/**
 * @brief 重置内部状态
 */
void Resampler::reset()
{
    m_delayLine.assign(m_delaySize, 0.0);
    m_delayPos = 0;
    m_phaseIndex = 0;
    m_sampleIndex = 0;
}

/**
 * @brief 获取理论输出长度
 */
int Resampler::expectedOutputLength(int inputLength) const
{
    if (!m_configured || m_M == 0) return 0;
    return static_cast<int>(static_cast<double>(inputLength) * m_L / m_M);
}

/** @brief 重置统计 */
void Resampler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设计多相抗混叠滤波器
 * 使用sinc原型滤波器 + Kaiser窗
 */
void Resampler::designPolyphaseFilter(int filterLength, double stopbandAtten)
{
    double cutoffNorm = 1.0 / static_cast<double>(qMax(m_L, m_M));
    double beta = kaiserBeta(stopbandAtten);

    /* 滤波器总长度 */
    int totalTaps = (filterLength > 0) ? filterLength : m_L * 32;
    totalTaps = qMax(m_L * 4, totalTaps);
    /* 对齐到m_L的倍数 */
    totalTaps = ((totalTaps + m_L - 1) / m_L) * m_L;

    m_tapsPerPhase = totalTaps / m_L;

    /* 计算原型滤波器系数 */
    QVector<double> proto(totalTaps, 0.0);
    int halfLen = totalTaps / 2;

    for (int n = 0; n < totalTaps; ++n) {
        double t = static_cast<double>(n) - static_cast<double>(halfLen);
        double win = kaiserWindow(n, totalTaps, beta);
        proto[n] = 2.0 * cutoffNorm * sinc(2.0 * cutoffNorm * t) * win;
    }

    /* 多相分解: 将proto分成L组 */
    m_polyPhase.resize(m_L);
    for (int phase = 0; phase < m_L; ++phase) {
        m_polyPhase[phase].resize(m_tapsPerPhase);
        for (int t = 0; t < m_tapsPerPhase; ++t) {
            int srcIdx = phase + t * m_L;
            m_polyPhase[phase][t] = (srcIdx < totalTaps)
                ? proto[srcIdx] : 0.0;
        }
    }
}

/**
 * @brief 计算最大公约数(欧几里得算法)
 */
int Resampler::gcd(int a, int b)
{
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

/**
 * @brief sinc函数: sin(pi*x) / (pi*x)
 */
double Resampler::sinc(double x)
{
    if (std::abs(x) < 1e-10) return 1.0;
    double px = M_PI * x;
    return std::sin(px) / px;
}

/**
 * @brief Kaiser窗函数
 */
double Resampler::kaiserWindow(int n, int N, double beta)
{
    double halfN = static_cast<double>(N - 1) / 2.0;
    double x = (static_cast<double>(n) - halfN) / halfN;
    double arg = std::sqrt(qMax(0.0, 1.0 - x * x));

    /* I0(beta * sqrt(1 - x^2)) / I0(beta) */
    double num = 0.0, den = 0.0, term = 1.0;
    double y = beta * arg;
    double sum = 1.0;
    for (int k = 1; k <= 30; ++k) {
        term *= (y / (2.0 * k)) * (y / (2.0 * k));
        sum += term;
        if (term < 1e-15) break;
    }
    num = sum;

    /* I0(beta) */
    sum = 1.0;
    term = 1.0;
    for (int k = 1; k <= 30; ++k) {
        term *= (beta / (2.0 * k)) * (beta / (2.0 * k));
        sum += term;
        if (term < 1e-15) break;
    }
    den = sum;

    return (den > 1e-30) ? num / den : 1.0;
}

/**
 * @brief 根据阻带衰减计算Kaiser窗beta参数
 */
double Resampler::kaiserBeta(double attenuation)
{
    if (attenuation > 50.0)
        return 0.1102 * (attenuation - 8.7);
    else if (attenuation >= 21.0)
        return 0.5842 * std::pow(attenuation - 21.0, 0.4)
            + 0.07886 * (attenuation - 21.0);
    else
        return 0.0;
}

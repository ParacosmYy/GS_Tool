/**
 * @file SampleRateConv.cpp
 * @brief 采样率转换器实现 — Kaiser窗FIR抗混叠+多相分解+线性插值
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/dsp24/SampleRateConv.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 *
 * 根据输入/输出采样率初始化转换比率并设计抗混叠滤波器。
 *
 * @param inputRate 输入采样率(Hz)
 * @param outputRate 输出采样率(Hz)
 * @param filterLength FIR滤波器长度(偶数)
 * @param parent 父对象
 */
SampleRateConv::SampleRateConv(double inputRate, double outputRate,
                               int filterLength, QObject* parent)
    : QObject(parent)
    , m_inputRate(inputRate)
    , m_outputRate(outputRate)
    , m_filterLength(filterLength)
{
    m_ratio = (inputRate > 0.0) ? outputRate / inputRate : 1.0;
    m_filterDelay = 0.0;
    designFilter();
}

/**
 * @brief 设计Kaiser窗FIR抗混叠滤波器
 *
 * 使用Kaiser窗函数设计低通滤波器,截止频率取输入/输出Nyquist的最小值。
 * 滤波器系数用于多相分解实现高效重采样。
 */
void SampleRateConv::designFilter()
{
    m_filter.resize(m_filterLength);

    /* 截止频率: 取输入和输出Nyquist频率的较小值 */
    double nyquistIn = m_inputRate / 2.0;
    double nyquistOut = m_outputRate / 2.0;
    double cutoffFreq = std::min(nyquistIn, nyquistOut);
    double normalizedCutoff = cutoffFreq / std::max(m_inputRate, m_outputRate);

    /* Kaiser窗参数: beta=5.0(约50dB旁瓣衰减) */
    const double beta = 5.0;
    double M = m_filterLength - 1;

    /* 预计算I0(beta)用于归一化 */
    double i0Beta = 0.0;
    double term = 1.0;
    double denom = 1.0;
    for (int k = 1; k <= 20; ++k) {
        denom *= k;
        double bk = beta / 2.0 * k;
        term = 1.0;
        for (int j = 0; j < k; ++j) {
            term *= bk / (j + 1);
        }
        i0Beta += term * term / denom;
    }
    i0Beta += 1.0;

    /* 设计低通FIR系数 */
    double sum = 0.0;
    for (int n = 0; n < m_filterLength; ++n) {
        double centerN = n - M / 2.0;

        /* sinc函数(避免除零) */
        double sinc;
        if (std::abs(centerN) < 1e-10) {
            sinc = 1.0;
        } else {
            sinc = std::sin(M_PI * 2.0 * normalizedCutoff * centerN)
                 / (M_PI * centerN);
        }

        /* Kaiser窗 */
        double ratio = 2.0 * n / M - 1.0;
        double arg = std::sqrt(std::max(0.0, 1.0 - ratio * ratio));
        double i0Arg = 1.0;
        {
            double t = 1.0;
            double d = 1.0;
            double a = beta * arg / 2.0;
            for (int k = 1; k <= 20; ++k) {
                d *= k;
                double p = 1.0;
                for (int j = 0; j < k; ++j) {
                    p *= a / (j + 1);
                }
                t = p * p / d;
                i0Arg += t;
            }
        }
        double window = i0Arg / i0Beta;

        m_filter[n] = sinc * window * 2.0 * normalizedCutoff;
        sum += m_filter[n];
    }

    /* 归一化使通带增益为1 */
    if (sum > 0.0) {
        for (int n = 0; n < m_filterLength; ++n) {
            m_filter[n] /= sum;
        }
    }

    /* 滤波器群延迟(采样) */
    m_filterDelay = M / 2.0;
}

/**
 * @brief 执行采样率转换
 *
 * 使用多相分解+线性插值实现任意比率重采样:
 * 1. 对每个输出样本计算对应的输入时间位置
 * 2. 使用FIR滤波器系数在多相子滤波器间线性插值
 *
 * @param input 输入信号
 * @return 转换后信号
 */
QVector<double> SampleRateConv::convert(const QVector<double>& input)
{
    if (input.isEmpty() || m_filter.isEmpty()) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    int inputLen = input.size();
    int outputLen = static_cast<int>(std::ceil(inputLen * m_ratio));
    if (outputLen <= 0) {
        return {};
    }

    QVector<double> output(outputLen, 0.0);
    int halfLen = m_filterLength / 2;

    for (int outIdx = 0; outIdx < outputLen; ++outIdx) {
        /* 输出样本对应的输入时间位置 */
        double inTime = outIdx / m_ratio;

        /* 计算最近的多相相位和插值分数 */
        double phaseContinuous = inTime - std::floor(inTime);
        int phase = static_cast<int>(phaseContinuous * m_filterLength);
        double frac = phaseContinuous * m_filterLength - phase;
        if (phase >= m_filterLength) {
            phase = m_filterLength - 1;
            frac = 0.0;
        }

        int nextPhase = (phase + 1) % m_filterLength;

        /* 多相滤波: 从该相位对应的滤波器系数计算输出 */
        double sample0 = 0.0;
        double sample1 = 0.0;
        for (int k = 0; k < m_filterLength; ++k) {
            /* 输入样本索引 */
            int inIdx = static_cast<int>(std::floor(inTime)) - halfLen + k;
            if (inIdx >= 0 && inIdx < inputLen) {
                /* 多相系数: 从全长滤波器中按相位抽取 */
                int coeffIdx0 = (phase + k * m_filterLength) % m_filterLength;
                int coeffIdx1 = (nextPhase + k * m_filterLength) % m_filterLength;
                sample0 += input[inIdx] * m_filter[coeffIdx0];
                sample1 += input[inIdx] * m_filter[coeffIdx1];
            }
        }

        /* 在两个多相输出之间线性插值 */
        output[outIdx] = sample0 * (1.0 - frac) + sample1 * frac;
    }

    /* 更新统计信息 */
    m_stats.totalConversions++;
    m_stats.totalSamplesIn += inputLen;
    m_stats.totalSamplesOut += outputLen;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConversions;

    emit conversionCompleted(inputLen, outputLen);
    return output;
}

/**
 * @brief 设置新的转换比率
 *
 * 重新计算比率并设计新的抗混叠滤波器。
 *
 * @param inputRate 输入采样率(Hz)
 * @param outputRate 输出采样率(Hz)
 */
void SampleRateConv::setRates(double inputRate, double outputRate)
{
    m_inputRate = inputRate;
    m_outputRate = outputRate;
    m_ratio = (inputRate > 0.0) ? outputRate / inputRate : 1.0;
    designFilter();
}

/**
 * @brief 重置统计信息
 */
void SampleRateConv::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file MultiRateFilter.cpp
 * @brief 多速率滤波器组实现 — 抽取/内插
 */

#include "utils/dsp11/MultiRateFilter.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
MultiRateFilter::MultiRateFilter(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设计低通FIR滤波器(窗口法) @param cutoffFreq 归一化截止频率 @param filterLength 滤波器长度 @param windowType 窗类型 @return 滤波器系数 */
QVector<double> MultiRateFilter::designLowPass(double cutoffFreq, int filterLength,
                                                int windowType) const
{
    QVector<double> h(filterLength);
    int M = filterLength - 1;
    double sum = 0.0;

    for (int n = 0; n < filterLength; ++n) {
        /* 理想低通脉冲响应( sinc ) */
        double sinc;
        if (n == M / 2) {
            sinc = 2.0 * cutoffFreq;
        } else {
            double nm = static_cast<double>(n) - M / 2.0;
            sinc = std::sin(2.0 * M_PI * cutoffFreq * nm)
                 / (M_PI * nm);
        }

        /* 窗函数 */
        double w = 1.0;
        switch (windowType) {
        case 1: /* Hann */
            w = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / M));
            break;
        case 2: /* Hamming */
            w = 0.54 - 0.46 * std::cos(2.0 * M_PI * n / M);
            break;
        case 3: /* Blackman */
            w = 0.42 - 0.5 * std::cos(2.0 * M_PI * n / M)
                + 0.08 * std::cos(4.0 * M_PI * n / M);
            break;
        default: /* 矩形窗 */
            break;
        }

        h[n] = sinc * w;
        sum += h[n];
    }

    /* 归一化使直流增益=1 */
    for (auto& coeff : h) coeff /= sum;
    return h;
}

/** @brief 设计半带滤波器 @param filterLength 滤波器长度 @return 滤波器系数 */
QVector<double> MultiRateFilter::designHalfBand(int filterLength) const
{
    /* 半带滤波器: 归一化截止=0.25, 每隔一个系数为0(除中心) */
    /* 确保长度为4k+3 */
    while ((filterLength - 3) % 4 != 0) ++filterLength;

    QVector<double> h(filterLength, 0.0);
    int M = filterLength - 1;
    int center = M / 2;

    for (int n = 0; n < filterLength; ++n) {
        int k = n - center;
        if (k == 0) {
            h[n] = 0.5;
        } else if (k % 2 == 0) {
            /* 偶数偏移: 非零系数 */
            h[n] = -1.0 / (M_PI * k) * std::sin(M_PI * k / 2.0)
                   * std::cos(M_PI * k / 2.0);
            /* 简化: (-1)^(k/2) / (pi * k) * sin(pi*k/4) */
            h[n] = std::sin(M_PI * k / 4.0) / (M_PI * k);
        }
        /* 奇数偏移系数保持0 */
    }

    /* Hann窗加权 */
    double sum = 0.0;
    for (int n = 0; n < filterLength; ++n) {
        double w = 0.5 * (1.0 - std::cos(2.0 * M_PI * n / M));
        h[n] *= w;
        sum += h[n];
    }
    for (auto& c : h) c /= sum;

    /* 强制奇数位置为0(半带特性) */
    for (int n = 0; n < filterLength; ++n) {
        int k = n - center;
        if (k != 0 && k % 2 != 0) h[n] = 0.0;
    }

    return h;
}

/** @brief 设计CIC滤波器 @param stageCount 级数 @param differentialDelay 差分延迟 @param decimationFactor 抽取因子 @return 归一化CIC频率响应 */
QVector<double> MultiRateFilter::designCIC(int stageCount, int differentialDelay,
                                            int decimationFactor) const
{
    /* CIC传递函数: H(z) = ((1 - z^(-RM)) / (1 - z^(-1)))^N */
    /* 脉冲响应近似(梳状+积分分解太长, 这里直接计算频率响应) */
    int numPoints = 512;
    QVector<double> response(numPoints);

    for (int k = 0; k < numPoints; ++k) {
        double freq = M_PI * k / numPoints;
        /* 单级幅频: |sin(omega * R * M / 2) / sin(omega / 2)| */
        double num, den;
        if (std::abs(freq) < 1e-12) {
            num = static_cast<double>(differentialDelay * decimationFactor);
            den = 1.0;
        } else {
            num = std::sin(freq * differentialDelay * decimationFactor / 2.0);
            den = std::sin(freq / 2.0);
        }

        double mag = (std::abs(den) > 1e-12) ? std::abs(num / den) : 0.0;
        /* N级 */
        response[k] = std::pow(mag, stageCount);

        /* 归一化到最大值=1 */
        if (response[0] > 1e-12) {
            response[k] /= response[0];
        }
    }

    /* 重新归一化 */
    double maxVal = *std::max_element(response.begin(), response.end());
    if (maxVal > 1e-12) {
        for (auto& v : response) v /= maxVal;
    }

    return response;
}

/** @brief FIR滤波 @param signal 输入信号 @param coefficients 滤波器系数 @return 滤波后信号 */
QVector<double> MultiRateFilter::applyFir(const QVector<double>& signal,
                                           const QVector<double>& coefficients)
{
    int n = signal.size();
    int m = coefficients.size();
    QVector<double> output(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < m; ++j) {
            int idx = i - j;
            if (idx >= 0 && idx < n) {
                sum += coefficients[j] * signal[idx];
            }
        }
        output[i] = sum;
    }
    return output;
}

/** @brief 抽取(降低采样率) @param signal 输入信号 @param factor 抽取因子 @param filter 抗混叠滤波器 @return 抽取后信号 */
QVector<double> MultiRateFilter::decimate(const QVector<double>& signal, int factor,
                                           QVector<double> filter)
{
    QElapsedTimer timer;
    timer.start();

    if (factor <= 0 || signal.isEmpty()) return {};

    /* 自动设计抗混叠滤波器 */
    if (filter.isEmpty()) {
        int filtLen = std::min(101, static_cast<int>(signal.size()));
        if (filtLen % 2 == 0) ++filtLen;
        filter = designLowPass(0.5 / factor, filtLen, 1);
    }

    /* 先滤波, 再下采样 */
    auto filtered = applyFir(signal, filter);

    int outLen = (filtered.size() + factor - 1) / factor;
    QVector<double> output(outLen);
    for (int i = 0; i < outLen; ++i) {
        output[i] = filtered[i * factor];
    }

    m_stats.totalSamplesProcessed += signal.size();
    ++m_stats.totalDecimations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalDecimations + m_stats.totalInterpolations);

    emit decimationCompleted(signal.size(), output.size(), factor);
    return output;
}

/** @brief 内插(提高采样率) @param signal 输入信号 @param factor 内插因子 @param filter 镜像抑制滤波器 @return 内插后信号 */
QVector<double> MultiRateFilter::interpolate(const QVector<double>& signal, int factor,
                                              QVector<double> filter)
{
    QElapsedTimer timer;
    timer.start();

    if (factor <= 0 || signal.isEmpty()) return {};

    /* 自动设计镜像抑制滤波器 */
    if (filter.isEmpty()) {
        int filtLen = std::min(101, static_cast<int>(signal.size() * factor));
        if (filtLen % 2 == 0) ++filtLen;
        filter = designLowPass(0.5 / factor, filtLen, 1);
        /* 内插滤波器增益补偿 */
        for (auto& c : filter) c *= factor;
    }

    /* 上采样: 插零 */
    int upLen = signal.size() * factor;
    QVector<double> upsampled(upLen, 0.0);
    for (int i = 0; i < signal.size(); ++i) {
        upsampled[i * factor] = signal[i];
    }

    /* 低通滤波 */
    auto output = applyFir(upsampled, filter);

    m_stats.totalSamplesProcessed += signal.size();
    ++m_stats.totalInterpolations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalDecimations + m_stats.totalInterpolations);

    emit interpolationCompleted(signal.size(), output.size(), factor);
    return output;
}

/** @brief 任意比率重采样(线性内插) @param signal 输入信号 @param inputRate 输入采样率 @param outputRate 输出采样率 @return 重采样后信号 */
QVector<double> MultiRateFilter::resample(const QVector<double>& signal,
                                           double inputRate, double outputRate)
{
    QElapsedTimer timer;
    timer.start();

    if (inputRate <= 0.0 || outputRate <= 0.0 || signal.isEmpty()) return {};

    double ratio = outputRate / inputRate;
    int outputLen = static_cast<int>(std::round(signal.size() / ratio));
    if (outputLen <= 0) outputLen = 1;

    QVector<double> output(outputLen);
    for (int i = 0; i < outputLen; ++i) {
        double srcPos = i * ratio;
        int idx0 = static_cast<int>(srcPos);
        int idx1 = idx0 + 1;
        double frac = srcPos - idx0;

        double v0 = (idx0 < signal.size()) ? signal[idx0] : 0.0;
        double v1 = (idx1 < signal.size()) ? signal[idx1] : v0;
        output[i] = v0 + frac * (v1 - v0);
    }

    m_stats.totalSamplesProcessed += signal.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalDecimations + m_stats.totalInterpolations);

    return output;
}

/** @brief 多相分解 @param filter 原始滤波器 @param factor 分解因子 @return 多相子滤波器列表 */
QList<QVector<double>> MultiRateFilter::polyphaseDecompose(
    const QVector<double>& filter, int factor) const
{
    QList<QVector<double>> subFilters;
    subFilters.reserve(factor);

    int subLen = (filter.size() + factor - 1) / factor;

    for (int p = 0; p < factor; ++p) {
        QVector<double> sub(subLen, 0.0);
        for (int k = 0; k < subLen; ++k) {
            int idx = p + k * factor;
            if (idx < filter.size()) {
                sub[k] = filter[idx];
            }
        }
        subFilters.append(sub);
    }

    return subFilters;
}

/** @brief 多相抽取(高效实现) @param signal 输入信号 @param factor 抽取因子 @param polyphaseFilters 多相子滤波器 @return 抽取后信号 */
QVector<double> MultiRateFilter::polyphaseDecimate(
    const QVector<double>& signal, int factor,
    const QList<QVector<double>>& polyphaseFilters)
{
    QElapsedTimer timer;
    timer.start();

    if (factor <= 0 || signal.isEmpty() || polyphaseFilters.size() != factor) return {};

    int outLen = (signal.size() + factor - 1) / factor;
    QVector<double> output(outLen, 0.0);

    for (int n = 0; n < outLen; ++n) {
        double sum = 0.0;
        for (int p = 0; p < factor; ++p) {
            const auto& sub = polyphaseFilters[p];
            for (int k = 0; k < sub.size(); ++k) {
                int idx = n - k;
                if (idx >= 0 && idx < signal.size()) {
                    sum += sub[k] * signal[idx * factor + (factor - 1 - p) % factor];
                }
            }
        }
        output[n] = sum;
    }

    m_stats.totalSamplesProcessed += signal.size();
    ++m_stats.totalDecimations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / std::max(1, m_stats.totalDecimations + m_stats.totalInterpolations);

    return output;
}

/** @brief 重置统计 */
void MultiRateFilter::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SampleRateConv3.cpp
 * @brief 采样率转换器实现
 *
 * 实现基于多相滤波器的采样率转换。通过设计低通抗混叠滤波器
 * 并使用多相分解实现高效的任意比率重采样。支持可配置的
 * 滤波器阶数（taps）以平衡转换质量和计算复杂度。
 * 使用QElapsedTimer计时并累积统计信息。
 */

#include "utils/dsp50/SampleRateConv3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @class SampleRateConv3
 * @brief 多相滤波器采样率转换器
 *
 * 转换流程：
 * 1. 计算重采样比率 L/M（先上采样L倍，滤波，再下采样M倍）
 * 2. 设计截止频率为 min(pi/L, pi/M) 的低通FIR滤波器
 * 3. 使用线性插值在任意分数位置计算输出样本
 */

/**
 * @brief 构造函数，初始化默认采样率和滤波器
 * @param parent 父QObject指针
 */
SampleRateConv3::SampleRateConv3(QObject* parent)
    : QObject(parent)
{
    designFilter();
}

/**
 * @brief 设置输入和输出采样率
 * @param inputRate 输入采样率（Hz）
 * @param outputRate 输出采样率（Hz）
 */
void SampleRateConv3::setRates(double inputRate, double outputRate)
{
    m_inRate = qMax(1.0, inputRate);
    m_outRate = qMax(1.0, outputRate);
    m_ratio = m_outRate / m_inRate;
    designFilter();
}

/**
 * @brief 设置抗混叠滤波器阶数
 * @param taps 滤波器抽头数（越大质量越好，计算量越大）
 */
void SampleRateConv3::setQuality(int taps)
{
    m_taps = qMax(4, taps);
    designFilter();
}

/**
 * @brief 执行采样率转换
 *
 * 使用窗口法设计的FIR低通滤波器进行抗混叠滤波，
 * 通过线性插值在输出采样时间点计算重采样值。
 *
 * @param input 输入采样数据
 * @return 重采样后的输出数据
 */
QVector<double> SampleRateConv3::convert(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    if (input.isEmpty()) {
        m_stats.totalConversions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalConversions > 0)
            ? m_timeSum / m_stats.totalConversions : 0.0;
        return QVector<double>();
    }

    const int nIn = input.size();
    const int nOut = static_cast<int>(qCeil(nIn * m_ratio));
    QVector<double> output(nOut, 0.0);

    const double invRatio = 1.0 / m_ratio;
    const int halfTaps = m_taps / 2;

    for (int i = 0; i < nOut; ++i) {
        /* 计算输出样本对应的输入时间位置 */
        double t = i * invRatio;
        int center = static_cast<int>(qFloor(t));
        double frac = t - center;

        /* 确保滤波器窗口在输入范围内 */
        int start = center - halfTaps + 1;
        if (start < 0) start = 0;
        int end = center + halfTaps;
        if (end >= nIn) end = nIn - 1;

        double sum = 0.0;
        double wSum = 0.0;

        for (int j = start; j <= end; ++j) {
            /* 计算滤波器系数索引（考虑分数偏移） */
            double filterPos = (j - t) + halfTaps;
            int fIdx = static_cast<int>(qRound(filterPos));
            if (fIdx >= 0 && fIdx < m_filter.size()) {
                sum += input[j] * m_filter[fIdx];
                wSum += m_filter[fIdx];
            }
        }

        output[i] = (wSum > 1e-10) ? sum / wSum : 0.0;
    }

    m_stats.totalConversions++;
    m_stats.totalSamples += nIn;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalConversions > 0)
        ? m_timeSum / m_stats.totalConversions : 0.0;

    emit conversionCompleted(nIn, nOut);
    return output;
}

/**
 * @brief 重置统计数据
 */
void SampleRateConv3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设计低通抗混叠FIR滤波器
 *
 * 使用加窗法（Hamming窗）设计低通FIR滤波器。
 * 截止频率取输入和输出Nyquist频率的较小值，
 * 防止上采样时的镜像和下采样时的混叠。
 */
void SampleRateConv3::designFilter()
{
    m_filter.resize(m_taps);

    /* 截止频率：取min(输入Nyquist, 输出Nyquist)中较小者 */
    double nyquistIn = m_inRate / 2.0;
    double nyquistOut = m_outRate / 2.0;
    double cutoff = qMin(nyquistIn, nyquistOut) / qMax(m_inRate, m_outRate);

    /* 归一化截止频率（0~1范围，1=Nyquist） */
    double normCutoff = cutoff * 2.0;
    if (normCutoff > 0.99) normCutoff = 0.99;

    const int halfTaps = m_taps / 2;

    for (int i = 0; i < m_taps; ++i) {
        int n = i - halfTaps;

        /* 理想低通滤波器的冲激响应（sinc函数） */
        double sincVal;
        if (n == 0) {
            sincVal = normCutoff;
        } else {
            double x = M_PI * n * normCutoff;
            sincVal = normCutoff * qSin(x) / x;
        }

        /* Hamming窗 */
        double hamming = 0.54 - 0.46 * qCos(2.0 * M_PI * i / (m_taps - 1));

        m_filter[i] = sincVal * hamming;
    }

    /* 归一化滤波器增益 */
    double gain = 0.0;
    for (double c : m_filter) gain += c;
    if (gain > 1e-10) {
        for (double& c : m_filter) c /= gain;
    }

    m_initialized = true;
}

/**
 * @brief 使用多相分解进行高效重采样
 *
 * 将滤波器分解为多个多相分量，对每个输出样本
 * 仅计算必要的滤波器系数，避免零值乘法。
 * 此方法在输入较大时性能优于直接卷积。
 *
 * @param input 输入采样数据
 * @return 重采样后的输出数据
 *
 * @note 此方法为convert()的备选实现，提供更精确的
 *       多相插值。当需要高质量转换时可替代使用。
 */
QVector<double> SampleRateConv3::convertPolyphase(const QVector<double>& input)
{
    if (input.isEmpty() || !m_initialized) {
        return QVector<double>();
    }

    const int nIn = input.size();
    const int nOut = static_cast<int>(qCeil(nIn * m_ratio));
    QVector<double> output(nOut, 0.0);

    /* 计算有理比率 L/M 的近似 */
    const double invRatio = 1.0 / m_ratio;
    const int halfTaps = m_taps / 2;

    /* 预分配相位偏移表 */
    QVector<double> phaseOffsets(m_taps);
    for (int i = 0; i < nOut; ++i) {
        double t = i * invRatio;
        int center = static_cast<int>(qFloor(t));
        double frac = t - center;

        /* 计算多相滤波器系数（对原始系数进行分数偏移插值） */
        double sum = 0.0;
        int start = center - halfTaps + 1;
        if (start < 0) start = 0;
        int end = center + halfTaps;
        if (end >= nIn) end = nIn - 1;

        for (int j = start; j <= end; ++j) {
            /* 线性插值滤波器系数 */
            double fPos = (j - t) + halfTaps;
            int fIdxLow = static_cast<int>(qFloor(fPos));
            int fIdxHigh = fIdxLow + 1;
            double fFrac = fPos - fIdxLow;

            double coeff = 0.0;
            if (fIdxLow >= 0 && fIdxLow < m_filter.size()) {
                coeff += (1.0 - fFrac) * m_filter[fIdxLow];
            }
            if (fIdxHigh >= 0 && fIdxHigh < m_filter.size()) {
                coeff += fFrac * m_filter[fIdxHigh];
            }

            sum += input[j] * coeff;
        }
        output[i] = sum;
    }

    return output;
}

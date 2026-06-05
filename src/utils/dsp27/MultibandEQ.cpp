/**
 * @file MultibandEQ.cpp
 * @brief 多频段均衡器实现 — 双二阶滤波/频段增益/频率响应
 */

#include "utils/dsp27/MultibandEQ.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
MultibandEQ::MultibandEQ(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void MultibandEQ::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    updateAllCoeffs();
}

/** @brief 添加频段 @param param 频段参数 @return 频段索引 */
int MultibandEQ::addBand(const BandParam& param)
{
    m_bands.append(param);
    m_coeffs.append(computeCoeffs(param));
    m_states.append(BiquadState{});
    return m_bands.size() - 1;
}

/** @brief 修改频段参数 @param index 频段索引 @param param 新参数 */
void MultibandEQ::setBandParam(int index, const BandParam& param)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index] = param;
    m_coeffs[index] = computeCoeffs(param);
    m_states[index] = BiquadState{};
}

/** @brief 移除频段 @param index 频段索引 */
void MultibandEQ::removeBand(int index)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands.removeAt(index);
    m_coeffs.removeAt(index);
    m_states.removeAt(index);
}

/** @brief 处理音频块 @param input 输入采样 @return 输出采样 */
QVector<double> MultibandEQ::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output = input;
    int bandCount = m_bands.size();

    for (int b = 0; b < bandCount; ++b) {
        if (!m_bands[b].enabled) continue;
        const BiquadCoeffs& c = m_coeffs[b];
        BiquadState& s = m_states[b];
        for (int i = 0; i < n; ++i) {
            output[i] = processBiquad(output[i], c, s);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.totalProcessCalls++;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalProcessCalls);

    emit processComplete(n);
    return output;
}

/** @brief 计算频率响应 @param freqCount 频率点数 @return (频率, 增益dB)对 */
QVector<QPair<double, double>> MultibandEQ::frequencyResponse(
    int freqCount) const
{
    QVector<QPair<double, double>> response;
    response.reserve(freqCount);

    double nyquist = m_sampleRate / 2.0;

    for (int i = 0; i < freqCount; ++i) {
        /* 对数频率分布: 20Hz ~ Nyquist */
        double t = static_cast<double>(i) / (freqCount - 1);
        double freq = 20.0 * qPow(nyquist / 20.0, t);
        double omega = 2.0 * M_PI * freq / m_sampleRate;
        double cosW = qCos(omega);
        double sinW = qSin(omega);

        /* 累积所有频段的增益 */
        double totalGainLin = 1.0;

        for (int b = 0; b < m_bands.size(); ++b) {
            if (!m_bands[b].enabled) continue;
            const BiquadCoeffs& c = m_coeffs[b];

            /* 计算传递函数 H(z) 在 z=e^(jw) 处的值 */
            double realNum = c.b0 + c.b1 * cosW + c.b2 * qCos(2.0 * omega);
            double imagNum = -(c.b1 * sinW + c.b2 * qSin(2.0 * omega));
            double realDen = 1.0 + c.a1 * cosW + c.a2 * qCos(2.0 * omega);
            double imagDen = -(c.a1 * sinW + c.a2 * qSin(2.0 * omega));

            double magSq = (realNum * realNum + imagNum * imagNum)
                         / (realDen * realDen + imagDen * imagDen);
            totalGainLin *= qSqrt(qMax(magSq, 0.0));
        }

        double gainDb = 20.0 * qLn(qMax(totalGainLin, 1e-30)) / qLn(10.0);
        response.append({freq, gainDb});
    }

    return response;
}

/** @brief 获取频段数 @return 频段数 */
int MultibandEQ::bandCount() const
{
    return m_bands.size();
}

/** @brief 获取频段参数 @param index 频段索引 @return 参数 */
MultibandEQ::BandParam MultibandEQ::bandParam(int index) const
{
    if (index < 0 || index >= m_bands.size()) return BandParam{};
    return m_bands[index];
}

/** @brief 计算双二阶滤波器系数 @param param 频段参数 @return 系数 */
MultibandEQ::BiquadCoeffs MultibandEQ::computeCoeffs(
    const BandParam& param) const
{
    BiquadCoeffs c;
    double omega = 2.0 * M_PI * param.frequency / m_sampleRate;
    double cosW = qCos(omega);
    double sinW = qSin(omega);
    double A = qPow(10.0, param.gain / 40.0); /* dB转线性 */
    double alpha = sinW / (2.0 * param.q);

    switch (param.type) {
    case FilterType::Peak: {
        double sqA = qSqrt(A);
        c.b0 = 1.0 + alpha * sqA;
        c.b1 = -2.0 * cosW;
        c.b2 = 1.0 - alpha * sqA;
        double a0 = 1.0 + alpha / sqA;
        c.a1 = 2.0 * cosW;
        c.a2 = -(1.0 - alpha / sqA);
        c.b0 /= a0; c.b1 /= a0; c.b2 /= a0;
        c.a1 /= a0; c.a2 /= a0;
        break;
    }
    case FilterType::LowShelf: {
        double sqA = qSqrt(A);
        double twoSqAAlpha = 2.0 * sqA * alpha;
        c.b0 = A * ((A + 1.0) - (A - 1.0) * cosW + twoSqAAlpha);
        c.b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW);
        c.b2 = A * ((A + 1.0) - (A - 1.0) * cosW - twoSqAAlpha);
        double a0 = (A + 1.0) + (A - 1.0) * cosW + twoSqAAlpha;
        c.a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW);
        c.a2 = (A + 1.0) + (A - 1.0) * cosW - twoSqAAlpha;
        c.b0 /= a0; c.b1 /= a0; c.b2 /= a0;
        c.a1 /= a0; c.a2 /= a0;
        break;
    }
    case FilterType::HighShelf: {
        double sqA = qSqrt(A);
        double twoSqAAlpha = 2.0 * sqA * alpha;
        c.b0 = A * ((A + 1.0) + (A - 1.0) * cosW + twoSqAAlpha);
        c.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW);
        c.b2 = A * ((A + 1.0) + (A - 1.0) * cosW - twoSqAAlpha);
        double a0 = (A + 1.0) - (A - 1.0) * cosW + twoSqAAlpha;
        c.a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW);
        c.a2 = (A + 1.0) - (A - 1.0) * cosW - twoSqAAlpha;
        c.b0 /= a0; c.b1 /= a0; c.b2 /= a0;
        c.a1 /= a0; c.a2 /= a0;
        break;
    }
    }
    return c;
}

/** @brief 双二阶滤波器处理单个采样 @param sample 输入 @param c 系数 @param s 状态 @return 输出 */
double MultibandEQ::processBiquad(double sample, const BiquadCoeffs& c,
                                   BiquadState& s) const
{
    double output = c.b0 * sample + c.b1 * s.x1 + c.b2 * s.x2
                  - c.a1 * s.y1 - c.a2 * s.y2;
    s.x2 = s.x1;
    s.x1 = sample;
    s.y2 = s.y1;
    s.y1 = output;
    return output;
}

/** @brief 更新所有滤波器系数 */
void MultibandEQ::updateAllCoeffs()
{
    m_coeffs.clear();
    for (int i = 0; i < m_bands.size(); ++i) {
        m_coeffs.append(computeCoeffs(m_bands[i]));
    }
}

/** @brief 重置统计 */
void MultibandEQ::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

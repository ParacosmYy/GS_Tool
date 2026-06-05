/**
 * @file ParametricEq.cpp
 * @brief 参数均衡器实现 — 多段双二阶IIR滤波处理
 */

#include "utils/dsp9/ParametricEq.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ParametricEq::ParametricEq(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void ParametricEq::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
    /* 重新计算所有频段的系数 */
    for (int i = 0; i < m_bands.size(); ++i) {
        m_bands[i].coeffs = computeCoefficients(m_bands[i].params);
    }
}

/** @brief 添加滤波段 @param band 滤波段参数 @return 段索引 */
int ParametricEq::addBand(const Band& band)
{
    InternalBand ib;
    ib.params = band;
    ib.coeffs = computeCoefficients(band);
    ib.state = BiquadState{};
    m_bands.append(ib);
    m_stats.totalBandUpdates++;
    return m_bands.size() - 1;
}

/** @brief 移除滤波段 @param index 段索引 */
void ParametricEq::removeBand(int index)
{
    if (index >= 0 && index < m_bands.size()) {
        m_bands.removeAt(index);
    }
}

/** @brief 更新滤波段参数 @param index 段索引 @param band 新参数 */
void ParametricEq::updateBand(int index, const Band& band)
{
    if (index >= 0 && index < m_bands.size()) {
        m_bands[index].params = band;
        m_bands[index].coeffs = computeCoefficients(band);
        m_bands[index].state = BiquadState{};
        m_stats.totalBandUpdates++;
        emit bandUpdated(index);
    }
}

/** @brief 获取所有频段 @return 频段列表 */
QList<ParametricEq::Band> ParametricEq::bands() const
{
    QList<Band> result;
    for (const auto& ib : m_bands) {
        result.append(ib.params);
    }
    return result;
}

/** @brief 处理单个采样点 @param sample 输入采样 @return 滤波后采样 */
double ParametricEq::processSample(double sample)
{
    double x = sample;
    for (int i = 0; i < m_bands.size(); ++i) {
        if (!m_bands[i].params.enabled) continue;

        const auto& c = m_bands[i].coeffs;
        auto& s = m_bands[i].state;

        /* 双二阶直接I型: y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2 */
        double y = c.b0 * x + c.b1 * s.x1 + c.b2 * s.x2
                   - c.a1 * s.y1 - c.a2 * s.y2;

        /* 更新延迟线 */
        s.x2 = s.x1;
        s.x1 = x;
        s.y2 = s.y1;
        s.y1 = y;
        x = y;
    }
    return x;
}

/** @brief 批量处理数据 @param input 输入数据 @return 滤波后数据 */
QVector<double> ParametricEq::processBuffer(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> output(input.size());
    for (int i = 0; i < input.size(); ++i) {
        output[i] = processSample(input[i]);
    }

    m_stats.totalFramesProcessed++;
    m_stats.totalSamplesProcessed += input.size();
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFramesProcessed);

    emit processingComplete(input.size());
    return output;
}

/** @brief 计算指定频段的频率响应 @param frequencies 频率数组 @param bandIndex 频段索引 @return 幅度响应(dB) */
QVector<double> ParametricEq::frequencyResponse(const QVector<double>& frequencies,
                                                 int bandIndex) const
{
    QVector<double> response(frequencies.size(), 0.0);
    if (bandIndex < 0 || bandIndex >= m_bands.size()) return response;

    const auto& c = m_bands[bandIndex].coeffs;
    for (int i = 0; i < frequencies.size(); ++i) {
        double w = 2.0 * M_PI * frequencies[i] / m_sampleRate;
        double cosW = qCos(w);
        double sinW = qSin(w);

        /* H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2) */
        double numRe = c.b0 + c.b1 * cosW + c.b2 * qCos(2.0 * w);
        double numIm = -(c.b1 * sinW + c.b2 * qSin(2.0 * w));
        double denRe = 1.0 + c.a1 * cosW + c.a2 * qCos(2.0 * w);
        double denIm = -(c.a1 * sinW + c.a2 * qSin(2.0 * w));

        double magSq = (numRe * numRe + numIm * numIm)
                       / (denRe * denRe + denIm * denIm);
        response[i] = 10.0 * qLn(magSq + 1e-30) / qLn(10.0);
    }
    return response;
}

/** @brief 计算总频率响应 @param frequencies 频率数组 @return 总幅度响应(dB) */
QVector<double> ParametricEq::totalFrequencyResponse(const QVector<double>& frequencies) const
{
    QVector<double> response(frequencies.size(), 0.0);
    for (int b = 0; b < m_bands.size(); ++b) {
        if (!m_bands[b].params.enabled) continue;
        QVector<double> bandResp = frequencyResponse(frequencies, b);
        for (int i = 0; i < frequencies.size(); ++i) {
            response[i] += bandResp[i]; /* dB域叠加 */
        }
    }
    return response;
}

/** @brief 重置所有滤波器状态(清空延迟线) */
void ParametricEq::resetState()
{
    for (auto& band : m_bands) {
        band.state = BiquadState{};
    }
}

/** @brief 重置统计 */
void ParametricEq::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 根据Band参数计算双二阶系数 @param band 滤波段参数 @return 双二阶系数 */
ParametricEq::BiquadCoeffs ParametricEq::computeCoefficients(const Band& band) const
{
    BiquadCoeffs c;
    double A = qPow(10.0, band.gainDb / 40.0);
    double w0 = 2.0 * M_PI * band.frequency / m_sampleRate;
    double cosW = qCos(w0);
    double sinW = qSin(w0);
    double alpha = sinW / (2.0 * band.q);

    switch (band.type) {
    case FilterType::Peaking:
        c.b0 = 1.0 + alpha * A;
        c.b1 = -2.0 * cosW;
        c.b2 = 1.0 - alpha * A;
        c.a1 = -2.0 * cosW;
        c.a2 = 1.0 - alpha / A;
        break;
    case FilterType::LowShelf: {
        double sqA = qSqrt(A);
        double beta = 2.0 * sqA * alpha;
        c.b0 = A * ((A + 1.0) - (A - 1.0) * cosW + beta);
        c.b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW);
        c.b2 = A * ((A + 1.0) - (A - 1.0) * cosW - beta);
        c.a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW);
        c.a2 = (A + 1.0) + (A - 1.0) * cosW - beta;
        break;
    }
    case FilterType::HighShelf: {
        double sqA = qSqrt(A);
        double beta = 2.0 * sqA * alpha;
        c.b0 = A * ((A + 1.0) + (A - 1.0) * cosW + beta);
        c.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW);
        c.b2 = A * ((A + 1.0) + (A - 1.0) * cosW - beta);
        c.a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW);
        c.a2 = (A + 1.0) - (A - 1.0) * cosW - beta;
        break;
    }
    case FilterType::Notch:
        c.b0 = 1.0;
        c.b1 = -2.0 * cosW;
        c.b2 = 1.0;
        c.a1 = -2.0 * cosW;
        c.a2 = 1.0 - alpha;
        break;
    }

    /* 归一化: 使a0=1 */
    double sqA = qSqrt(A);
    double betaVal = 2.0 * sqA * alpha;

    if (band.type == FilterType::Peaking) {
        double denom = 1.0 + alpha / A;
        c.b0 /= denom; c.b1 /= denom; c.b2 /= denom;
        c.a1 /= denom; c.a2 /= denom;
    } else if (band.type == FilterType::Notch) {
        double denom = 1.0 + alpha;
        c.b0 /= denom; c.b1 /= denom; c.b2 /= denom;
        c.a1 /= denom; c.a2 /= denom;
    } else {
        /* LowShelf / HighShelf */
        double a0 = (A + 1.0) + (A - 1.0) * cosW + betaVal;
        c.b0 /= a0; c.b1 /= a0; c.b2 /= a0;
        c.a1 /= a0; c.a2 /= a0;
    }

    return c;
}

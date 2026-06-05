/**
 * @file DataResampler.cpp
 * @brief 数据重采样引擎实现 — 4种插值方法
 */

#include "utils/resample/DataResampler.h"

#include <QtMath>
#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
DataResampler::DataResampler(QObject* parent)
    : QObject(parent)
    , m_method(ResampleMethod::Linear)
{
}

void DataResampler::setMethod(ResampleMethod method) { m_method = method; }
void DataResampler::setConfig(const ResampleConfig& config) { m_config = config; }

/** @brief 重采样 @param data 输入 @return 重采样数据 */
QVector<double> DataResampler::resample(const QVector<double>& data)
{
    if (data.isEmpty() || m_config.sourceRate <= 0) return {};

    double ratio = m_config.targetRate / m_config.sourceRate;
    int outputSize = qMax(1, static_cast<int>(data.size() * ratio));

    return resampleToSize(data, outputSize);
}

/** @brief 重采样到指定长度 @param data 数据 @param targetSize 目标长度 @return 重采样数据 */
QVector<double> DataResampler::resampleToSize(
    const QVector<double>& data, int targetSize)
{
    if (data.isEmpty() || targetSize < 1) return {};
    if (targetSize == data.size()) return data;

    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    result.reserve(targetSize);

    int n = data.size();
    double ratio = static_cast<double>(n - 1) / qMax(1, targetSize - 1);

    for (int i = 0; i < targetSize; ++i) {
        double srcIdx = i * ratio;

        double value = 0.0;
        switch (m_method) {
        case ResampleMethod::Nearest:
            value = interpolateNearest(data, srcIdx);
            break;
        case ResampleMethod::Linear:
            value = interpolateLinear(data, srcIdx);
            break;
        case ResampleMethod::Cubic:
            value = interpolateCubic(data, srcIdx);
            break;
        case ResampleMethod::Sinc:
            value = interpolateSinc(data, srcIdx);
            break;
        }
        result.append(value);
    }

    m_stats.totalSamplesProcessed += static_cast<quint64>(data.size());
    m_stats.totalOutputSamples += static_cast<quint64>(result.size());
    ++m_stats.totalResamples;

    qint64 elapsed = timer.elapsed();
    double latencyUs = static_cast<double>(elapsed) * 1000.0;
    if (latencyUs > m_stats.peakLatencyUs) {
        m_stats.peakLatencyUs = latencyUs;
    }

    emit resampled(data.size(), result.size());
    return result;
}

void DataResampler::resetStatistics() { m_stats = Stats{}; }

/** @brief 最近邻插值 @param data 数据 @param idx 浮点索引 @return 插值 */
double DataResampler::interpolateNearest(
    const QVector<double>& data, double idx) const
{
    int i = qBound(0, static_cast<int>(std::round(idx)), data.size() - 1);
    return data[i];
}

/** @brief 线性插值 @param data 数据 @param idx 浮点索引 @return 插值 */
double DataResampler::interpolateLinear(
    const QVector<double>& data, double idx) const
{
    int n = data.size();
    if (n == 1) return data[0];

    int i0 = qBound(0, static_cast<int>(std::floor(idx)), n - 2);
    int i1 = i0 + 1;
    double frac = idx - static_cast<double>(i0);

    return data[i0] * (1.0 - frac) + data[i1] * frac;
}

/** @brief 三次Catmull-Rom插值 @param data 数据 @param idx 浮点索引 @return 插值 */
double DataResampler::interpolateCubic(
    const QVector<double>& data, double idx) const
{
    int n = data.size();
    if (n < 4) return interpolateLinear(data, idx);

    int i1 = qBound(1, static_cast<int>(std::round(idx)), n - 2);
    int i0 = qMax(0, i1 - 1);
    int i2 = qMin(n - 1, i1 + 1);
    int i3 = qMin(n - 1, i1 + 2);

    double t = idx - static_cast<int>(std::floor(idx));
    double t2 = t * t;
    double t3 = t2 * t;

    /* Catmull-Rom系数 */
    double p0 = data[i0], p1 = data[i1], p2 = data[i2], p3 = data[i3];
    return 0.5 * ((2.0 * p1) +
        (-p0 + p2) * t +
        (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 +
        (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
}

/** @brief Sinc插值(Lanczos窗) @param data 数据 @param idx 浮点索引 @return 插值 */
double DataResampler::interpolateSinc(
    const QVector<double>& data, double idx) const
{
    int n = data.size();
    int taps = m_config.sincTaps;
    int iCenter = static_cast<int>(std::round(idx));

    double sum = 0.0;
    double wSum = 0.0;

    for (int i = iCenter - taps; i <= iCenter + taps; ++i) {
        if (i < 0 || i >= n) continue;
        double d = idx - static_cast<double>(i);
        double s = sinc(d);

        /* Lanczos窗 */
        double x = d / taps;
        double w = (qFuzzyIsNull(d)) ? 1.0 : sinc(x);

        sum += data[i] * s * w;
        wSum += s * w;
    }

    if (qFuzzyIsNull(wSum)) {
        return data[qBound(0, iCenter, n - 1)];
    }
    return sum / wSum;
}

/** @brief Sinc函数 @param x 输入 @return sinc(x) */
double DataResampler::sinc(double x)
{
    if (qFuzzyIsNull(x)) return 1.0;
    double px = M_PI * x;
    return std::sin(px) / px;
}

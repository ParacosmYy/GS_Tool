/**
 * @file DataQuantizer.cpp
 * @brief 数据量化引擎实现 — 均匀/对数/μ律/A律量化
 */

#include "utils/quantize/DataQuantizer.h"

#include <QtMath>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
DataQuantizer::DataQuantizer(QObject* parent)
    : QObject(parent)
    , m_method(QuantizeMethod::Uniform)
    , m_levels(256)
    , m_errorSum(0.0)
    , m_mseSum(0.0)
{
}

void DataQuantizer::setMethod(QuantizeMethod method) { m_method = method; }

/** @brief 设置量化配置 @param config 配置 */
void DataQuantizer::setConfig(const QuantizeConfig& config)
{
    m_config = config;
    m_levels = 1 << qBound(1, config.bits, 32);
}

/** @brief 量化单个值 @param value 输入 @return 编码 */
int DataQuantizer::quantize(double value)
{
    int code = 0;
    switch (m_method) {
    case QuantizeMethod::Uniform:
        code = quantizeUniform(value);
        break;
    case QuantizeMethod::Logarithmic:
        code = quantizeLog(value);
        break;
    case QuantizeMethod::MuLaw:
        code = quantizeMuLaw(value);
        break;
    case QuantizeMethod::ALaw:
        code = quantizeALaw(value);
        break;
    }

    ++m_stats.totalValuesQuantized;
    double reconstructed = dequantize(code);
    double error = value - reconstructed;
    m_errorSum += qAbs(error);
    m_mseSum += error * error;
    m_stats.averageError = m_errorSum
        / static_cast<double>(m_stats.totalValuesQuantized);
    m_stats.mse = m_mseSum
        / static_cast<double>(m_stats.totalValuesQuantized);

    emit quantized(value, code, reconstructed);
    return code;
}

/** @brief 反量化 @param code 编码 @return 重建值 */
double DataQuantizer::dequantize(int code) const
{
    switch (m_method) {
    case QuantizeMethod::Uniform:     return dequantizeUniform(code);
    case QuantizeMethod::Logarithmic: return dequantizeLog(code);
    case QuantizeMethod::MuLaw:       return dequantizeMuLaw(code);
    case QuantizeMethod::ALaw:        return dequantizeALaw(code);
    }
    return 0.0;
}

/** @brief 批量量化 @param data 数据 @return 量化后数据 */
QVector<double> DataQuantizer::quantizeBatch(const QVector<double>& data)
{
    QVector<double> result;
    result.reserve(data.size());
    for (double v : data) {
        int code = quantize(v);
        result.append(dequantize(code));
    }

    ++m_stats.totalBatchesProcessed;

    /* 计算SNR */
    double snr = getSnr(data, result);
    if (snr > m_stats.peakSnr) m_stats.peakSnr = snr;

    return result;
}

/** @brief 计算SNR @param original 原始 @param quantized 量化 @return SNR(dB) */
double DataQuantizer::getSnr(const QVector<double>& original,
                              const QVector<double>& quantized) const
{
    int n = qMin(original.size(), quantized.size());
    if (n == 0) return 0.0;

    double signalPower = 0.0, noisePower = 0.0;
    for (int i = 0; i < n; ++i) {
        signalPower += original[i] * original[i];
        double err = original[i] - quantized[i];
        noisePower += err * err;
    }

    if (noisePower < 1e-20) return 120.0;
    return 10.0 * std::log10(signalPower / noisePower);
}

void DataQuantizer::resetStatistics()
{
    m_stats = Stats{};
    m_errorSum = 0.0;
    m_mseSum = 0.0;
}

/** @brief 均匀量化 @param value 输入 @return 编码 */
int DataQuantizer::quantizeUniform(double value) const
{
    double range = m_config.maxVal - m_config.minVal;
    if (range <= 0) return 0;
    double t = (value - m_config.minVal) / range;
    int code = static_cast<int>(t * m_levels);
    return qBound(0, code, m_levels - 1);
}

/** @brief 对数量化 @param value 输入 @return 编码 */
int DataQuantizer::quantizeLog(double value) const
{
    double sign = (value >= 0) ? 1.0 : -1.0;
    double absVal = std::fabs(value);
    if (absVal < 1e-10) return m_levels / 2;
    double logVal = std::log10(1.0 + absVal);
    double range = m_config.maxVal - m_config.minVal;
    double t = logVal / std::log10(1.0 + range);
    int code = static_cast<int>(t * m_levels / 2);
    if (sign < 0) code = m_levels - 1 - code;
    return qBound(0, code, m_levels - 1);
}

/** @brief μ律量化 @param value 输入 @return 编码 */
int DataQuantizer::quantizeMuLaw(double value) const
{
    const double MU = 255.0;
    double range = m_config.maxVal - m_config.minVal;
    if (range <= 0) return 0;
    double normalized = (value - m_config.minVal) / range * 2.0 - 1.0;
    double sign = (normalized >= 0) ? 1.0 : -1.0;
    double absV = std::fabs(normalized);
    double compressed = sign * std::log(1.0 + MU * absV) / std::log(1.0 + MU);
    int code = static_cast<int>((compressed + 1.0) / 2.0 * m_levels);
    return qBound(0, code, m_levels - 1);
}

/** @brief A律量化 @param value 输入 @return 编码 */
int DataQuantizer::quantizeALaw(double value) const
{
    const double A = 87.6;
    double range = m_config.maxVal - m_config.minVal;
    if (range <= 0) return 0;
    double normalized = (value - m_config.minVal) / range * 2.0 - 1.0;
    double sign = (normalized >= 0) ? 1.0 : -1.0;
    double absV = std::fabs(normalized);
    double compressed;
    if (absV < 1.0 / A) {
        compressed = sign * A * absV / (1.0 + std::log(A));
    } else {
        compressed = sign * (1.0 + std::log(A * absV)) / (1.0 + std::log(A));
    }
    int code = static_cast<int>((compressed + 1.0) / 2.0 * m_levels);
    return qBound(0, code, m_levels - 1);
}

/** @brief 均匀反量化 @param code 编码 @return 重建值 */
double DataQuantizer::dequantizeUniform(int code) const
{
    double range = m_config.maxVal - m_config.minVal;
    return m_config.minVal + (static_cast<double>(code) + 0.5)
        / m_levels * range;
}

/** @brief 对数反量化 @param code 编码 @return 重建值 */
double DataQuantizer::dequantizeLog(int code) const
{
    double range = m_config.maxVal - m_config.minVal;
    double t = static_cast<double>(code) / m_levels;
    return std::pow(10.0, t * std::log10(1.0 + range)) - 1.0;
}

/** @brief μ律反量化 @param code 编码 @return 重建值 */
double DataQuantizer::dequantizeMuLaw(int code) const
{
    const double MU = 255.0;
    double compressed = static_cast<double>(code) / m_levels * 2.0 - 1.0;
    double sign = (compressed >= 0) ? 1.0 : -1.0;
    double absC = std::fabs(compressed);
    double expanded = sign * (std::pow(1.0 + MU, absC) - 1.0) / MU;
    double range = m_config.maxVal - m_config.minVal;
    return m_config.minVal + (expanded + 1.0) / 2.0 * range;
}

/** @brief A律反量化 @param code 编码 @return 重建值 */
double DataQuantizer::dequantizeALaw(int code) const
{
    const double A = 87.6;
    double compressed = static_cast<double>(code) / m_levels * 2.0 - 1.0;
    double sign = (compressed >= 0) ? 1.0 : -1.0;
    double absC = std::fabs(compressed);
    double expanded;
    if (absC < 1.0 / (1.0 + std::log(A))) {
        expanded = sign * absC * (1.0 + std::log(A)) / A;
    } else {
        expanded = sign * std::exp(absC * (1.0 + std::log(A)) - 1.0) / A;
    }
    double range = m_config.maxVal - m_config.minVal;
    return m_config.minVal + (expanded + 1.0) / 2.0 * range;
}

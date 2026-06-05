/**
 * @file DataNormalizer.cpp
 * @brief 数据归一化引擎实现 — MinMax/Z-Score/Decimal/Log/Vector归一化
 */

#include "utils/normalize/DataNormalizer.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/* ── 构造/重置 ── */

/** @brief 构造函数，默认MinMax方法，目标范围[0,1] */
DataNormalizer::DataNormalizer(QObject* parent)
    : QObject(parent)
    , m_method(NormalizeMethod::MinMax)
    , m_targetMin(0.0)
    , m_targetMax(1.0)
    , m_dataMin(0.0)
    , m_dataMax(1.0)
    , m_mean(0.0)
    , m_stddev(1.0)
    , m_scaleFactor(1.0)
    , m_vectorNorm(1.0)
    , m_paramsValid(false)
{
}

/** @brief 设置归一化方法 @param method 归一化方法 */
void DataNormalizer::setMethod(NormalizeMethod method)
{
    m_method = method;
    m_paramsValid = false;
}

/** @brief 设置目标范围(仅MinMax方法使用) @param min 目标最小值 @param max 目标最大值 */
void DataNormalizer::setTargetRange(double min, double max)
{
    m_targetMin = min;
    m_targetMax = max;
}

/** @brief 重置归一化参数，保持统计不变 */
void DataNormalizer::reset()
{
    m_dataMin     = 0.0;
    m_dataMax     = 1.0;
    m_mean        = 0.0;
    m_stddev      = 1.0;
    m_scaleFactor = 1.0;
    m_vectorNorm  = 1.0;
    m_paramsValid = false;
}

/** @brief 重置所有统计计数器 */
void DataNormalizer::resetStatistics()
{
    m_stats = Stats{};
}

/* ── 公共接口 ── */

/** @brief 归一化单个值，根据当前方法分派 @param value 输入值 @return 归一化值 */
double DataNormalizer::normalize(double value)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;
    switch (m_method) {
    case NormalizeMethod::MinMax:
        result = normalizeMinMax(value);
        break;
    case NormalizeMethod::ZScore:
        result = normalizeZScore(value);
        break;
    case NormalizeMethod::DecimalScaling:
        result = normalizeDecimal(value);
        break;
    case NormalizeMethod::LogTransform:
        result = normalizeLog(value);
        break;
    case NormalizeMethod::VectorNorm:
        result = normalizeVector(value);
        break;
    }

    /* 更新统计: 峰值追踪 */
    const qreal absIn  = qAbs(value);
    const qreal absOut = qAbs(result);
    if (absIn  > m_stats.peakInputValue)  m_stats.peakInputValue  = absIn;
    if (absOut > m_stats.peakOutputValue) m_stats.peakOutputValue = absOut;
    ++m_stats.totalValuesNormalized;

    emit normalized(value, result);
    return result;
}

/** @brief 批量归一化: 先计算参数，再逐值归一化 @param data 输入数据 @return 归一化数据 */
QVector<double> DataNormalizer::normalizeBatch(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return {};
    }

    /* 先从批量数据中计算归一化参数 */
    updateParams(data);

    QVector<double> result;
    result.reserve(data.size());
    for (double val : data) {
        double norm = 0.0;
        switch (m_method) {
        case NormalizeMethod::MinMax:        norm = normalizeMinMax(val); break;
        case NormalizeMethod::ZScore:        norm = normalizeZScore(val); break;
        case NormalizeMethod::DecimalScaling: norm = normalizeDecimal(val); break;
        case NormalizeMethod::LogTransform:  norm = normalizeLog(val); break;
        case NormalizeMethod::VectorNorm:    norm = normalizeVector(val); break;
        }
        result.append(norm);

        /* 峰值追踪 */
        if (qAbs(val)  > m_stats.peakInputValue)  m_stats.peakInputValue  = qAbs(val);
        if (qAbs(norm) > m_stats.peakOutputValue) m_stats.peakOutputValue = qAbs(norm);
    }

    m_stats.totalValuesNormalized += static_cast<quint64>(data.size());
    ++m_stats.totalBatchesProcessed;

    emit batchNormalized(data.size());
    return result;
}

/** @brief 反归一化(仅MinMax方法可逆) @param value 归一化后的值 @return 原始值估计 */
double DataNormalizer::denormalize(double value) const
{
    if (!m_paramsValid) return value;

    const double range = m_dataMax - m_dataMin;
    if (qFuzzyIsNull(range)) return m_dataMin;

    /* 逆映射: value从[targetMin, targetMax]回到[dataMin, dataMax] */
    const double targetRange = m_targetMax - m_targetMin;
    if (qFuzzyIsNull(targetRange)) return m_dataMin;

    const double t = (value - m_targetMin) / targetRange;
    return m_dataMin + t * range;
}

/* ── 私有归一化方法 ── */

/** @brief MinMax归一化: 映射到[targetMin, targetRange] @param value 输入值 @return 归一化值 */
double DataNormalizer::normalizeMinMax(double value) const
{
    if (!m_paramsValid) return value;

    const double range = m_dataMax - m_dataMin;
    if (qFuzzyIsNull(range)) return (m_targetMin + m_targetMax) / 2.0;

    const double t = (value - m_dataMin) / range;
    return m_targetMin + t * (m_targetMax - m_targetMin);
}

/** @brief Z-Score标准化: (value - mean) / stddev @param value 输入值 @return 标准化值 */
double DataNormalizer::normalizeZScore(double value) const
{
    if (!m_paramsValid) return 0.0;
    if (qFuzzyIsNull(m_stddev)) return 0.0;
    return (value - m_mean) / m_stddev;
}

/** @brief 小数定标归一化: value / 10^j，j为使|结果|<1的最小整数 @param value 输入值 @return 归一化值 */
double DataNormalizer::normalizeDecimal(double value) const
{
    if (!m_paramsValid) return value;
    if (qFuzzyIsNull(m_scaleFactor)) return 0.0;
    return value / m_scaleFactor;
}

/** @brief 对数变换: ln(value)或ln(value+1)处理非正数 @param value 输入值 @return 变换值 */
double DataNormalizer::normalizeLog(double value) const
{
    if (value <= 0.0) {
        /* 对非正值使用ln(value+1)偏移，避免数学域错误 */
        const double shifted = value + 1.0;
        if (shifted <= 0.0) {
            /* 即使偏移后仍<=0，返回0并记录错误 */
            return 0.0;
        }
        return std::log(shifted);
    }
    return std::log(value);
}

/** @brief 向量L2范数归一化: value / ||vector|| @param value 输入值 @return 归一化值 */
double DataNormalizer::normalizeVector(double value) const
{
    if (!m_paramsValid) return value;
    if (qFuzzyIsNull(m_vectorNorm)) return 0.0;
    return value / m_vectorNorm;
}

/* ── 参数计算 ── */

/** @brief 从批量数据计算min/max/mean/stddev/scaleFactor/vectorNorm */
void DataNormalizer::updateParams(const QVector<double>& data)
{
    if (data.isEmpty()) return;

    const int n = data.size();

    /* 第一遍: 计算min/max/sum */
    double dMin = data[0];
    double dMax = data[0];
    double sum  = data[0];
    double sumSq = 0.0;

    for (int i = 1; i < n; ++i) {
        if (data[i] < dMin) dMin = data[i];
        if (data[i] > dMax) dMax = data[i];
        sum += data[i];
    }

    m_dataMin = dMin;
    m_dataMax = dMax;
    m_mean    = sum / static_cast<double>(n);

    /* 第二遍: 计算标准差 */
    for (int i = 0; i < n; ++i) {
        const double diff = data[i] - m_mean;
        sumSq += diff * diff;
    }
    m_stddev = std::sqrt(sumSq / static_cast<double>(n));
    if (qFuzzyIsNull(m_stddev)) m_stddev = 1.0;

    /* 小数定标因子: 10^j，j为使 max|value|/10^j < 1 的最小整数 */
    const double absMax = qMax(qAbs(dMin), qAbs(dMax));
    if (qFuzzyIsNull(absMax)) {
        m_scaleFactor = 1.0;
    } else {
        int j = static_cast<int>(std::ceil(std::log10(absMax)));
        if (j < 1) j = 1;
        m_scaleFactor = std::pow(10.0, static_cast<double>(j));
    }

    /* 向量L2范数: sqrt(sum(value_i^2)) */
    double sqSum = 0.0;
    for (int i = 0; i < n; ++i) {
        sqSum += data[i] * data[i];
    }
    m_vectorNorm = std::sqrt(sqSum);
    if (qFuzzyIsNull(m_vectorNorm)) m_vectorNorm = 1.0;

    m_paramsValid = true;
}

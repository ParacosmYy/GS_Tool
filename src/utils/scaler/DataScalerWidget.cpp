/**
 * @file DataScalerWidget.cpp
 * @brief 数据缩放/标定组件实现 — 线性/多项式/查表/指数标定
 */

#include "utils/scaler/DataScalerWidget.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
DataScalerWidget::DataScalerWidget(QObject* parent)
    : QObject(parent)
    , m_mode(ScaleMode::Linear)
    , m_slope(1.0)
    , m_offset(0.0)
    , m_expA(1.0)
    , m_expB(1.0)
    , m_expC(0.0)
    , m_lastRaw(0.0)
    , m_lastScaled(0.0)
{
}

/** @brief 设置线性标定参数 @param slope 斜率 @param offset 偏移 */
void DataScalerWidget::setLinearCalibration(double slope, double offset)
{
    m_slope = slope;
    m_offset = offset;
    m_mode = ScaleMode::Linear;
    ++m_stats.totalCalibrations;
}

/** @brief 设置自定义标定点(查表模式) @param points 标定点列表 */
void DataScalerWidget::setCustomCalibration(const QList<CalibrationPoint>& points)
{
    m_lookupTable = points;
    /* 按原始值升序排序，便于二分查找和插值 */
    std::sort(m_lookupTable.begin(), m_lookupTable.end(),
              [](const CalibrationPoint& a, const CalibrationPoint& b) {
                  return a.raw < b.raw;
              });
    m_mode = ScaleMode::LookupTable;
    ++m_stats.totalCalibrations;
}

/** @brief 设置多项式系数 @param coeffs 系数数组 */
void DataScalerWidget::setPolynomialCoeffs(const QVector<double>& coeffs)
{
    m_polyCoeffs = coeffs;
    m_mode = ScaleMode::Polynomial;
    ++m_stats.totalCalibrations;
}

/** @brief 设置指数标定参数 @param a 系数a @param b 系数b @param c 偏移c */
void DataScalerWidget::setExponentialParams(double a, double b, double c)
{
    m_expA = a;
    m_expB = b;
    m_expC = c;
    m_mode = ScaleMode::Exponential;
    ++m_stats.totalCalibrations;
}

/** @brief 设置当前标定模式 @param mode 标定模式 */
void DataScalerWidget::setScaleMode(ScaleMode mode)
{
    m_mode = mode;
}

/** @brief 转换单个原始值 @param rawValue 原始ADC值 @return 工程值 */
double DataScalerWidget::convert(double rawValue)
{
    double scaled = 0.0;
    switch (m_mode) {
    case ScaleMode::Linear:
        scaled = convertLinear(rawValue);
        break;
    case ScaleMode::Polynomial:
        scaled = convertPolynomial(rawValue);
        break;
    case ScaleMode::LookupTable:
        scaled = convertLookup(rawValue);
        break;
    case ScaleMode::Exponential:
        scaled = convertExponential(rawValue);
        break;
    }

    m_lastRaw = rawValue;
    m_lastScaled = scaled;

    /* 更新统计 */
    ++m_stats.totalConversions;
    if (rawValue > m_stats.peakRawValue) {
        m_stats.peakRawValue = rawValue;
    }
    if (m_stats.totalConversions == 1) {
        m_stats.minValue = scaled;
        m_stats.maxValue = scaled;
    } else {
        if (scaled < m_stats.minValue) m_stats.minValue = scaled;
        if (scaled > m_stats.maxValue) m_stats.maxValue = scaled;
    }

    emit valueConverted(rawValue, scaled);
    return scaled;
}

/** @brief 批量转换 @param rawValues 原始值列表 @return 工程值列表 */
QList<double> DataScalerWidget::batchConvert(const QList<double>& rawValues)
{
    QList<double> results;
    results.reserve(rawValues.size());
    for (double raw : rawValues) {
        /* 不逐个emit信号，在批量模式下减少信号开销 */
        double scaled = 0.0;
        switch (m_mode) {
        case ScaleMode::Linear:
            scaled = convertLinear(raw);
            break;
        case ScaleMode::Polynomial:
            scaled = convertPolynomial(raw);
            break;
        case ScaleMode::LookupTable:
            scaled = convertLookup(raw);
            break;
        case ScaleMode::Exponential:
            scaled = convertExponential(raw);
            break;
        }
        results.append(scaled);

        ++m_stats.totalConversions;
        if (raw > m_stats.peakRawValue) m_stats.peakRawValue = raw;
        if (m_stats.totalConversions == 1 || scaled < m_stats.minValue) {
            m_stats.minValue = scaled;
        }
        if (scaled > m_stats.maxValue) m_stats.maxValue = scaled;
    }
    if (!results.isEmpty()) {
        m_lastRaw = rawValues.last();
        m_lastScaled = results.last();
    }
    return results;
}

/** @brief 重置所有统计计数器 */
void DataScalerWidget::resetStatistics()
{
    m_stats = Stats{};
    m_lastRaw = 0.0;
    m_lastScaled = 0.0;
}

/* ── 私有转换方法 ── */

/** @brief 线性转换: y = slope * x + offset */
double DataScalerWidget::convertLinear(double raw) const
{
    return raw * m_slope + m_offset;
}

/** @brief 多项式转换: y = sum(coeffs[i] * x^i) */
double DataScalerWidget::convertPolynomial(double raw) const
{
    if (m_polyCoeffs.isEmpty()) return 0.0;
    double result = 0.0;
    double power  = 1.0;
    for (double coeff : m_polyCoeffs) {
        result += coeff * power;
        power  *= raw;
    }
    return result;
}

/** @brief 查表转换: 线性插值 */
double DataScalerWidget::convertLookup(double raw) const
{
    if (m_lookupTable.isEmpty()) return raw;

    /* 外推: 低于最低点 */
    if (raw <= m_lookupTable.first().raw) {
        if (m_lookupTable.size() >= 2) {
            const auto& p0 = m_lookupTable[0];
            const auto& p1 = m_lookupTable[1];
            double ratio = (p1.raw != p0.raw)
                           ? (raw - p0.raw) / (p1.raw - p0.raw) : 0.0;
            return p0.scaled + ratio * (p1.scaled - p0.scaled);
        }
        return m_lookupTable.first().scaled;
    }
    /* 外推: 高于最高点 */
    if (raw >= m_lookupTable.last().raw) {
        if (m_lookupTable.size() >= 2) {
            int n = m_lookupTable.size();
            const auto& pn1 = m_lookupTable[n - 2];
            const auto& pn0 = m_lookupTable[n - 1];
            double ratio = (pn0.raw != pn1.raw)
                           ? (raw - pn1.raw) / (pn0.raw - pn1.raw) : 1.0;
            return pn1.scaled + ratio * (pn0.scaled - pn1.scaled);
        }
        return m_lookupTable.last().scaled;
    }
    /* 二分查找插值区间 */
    int lo = 0, hi = m_lookupTable.size() - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (m_lookupTable[mid].raw <= raw) lo = mid;
        else hi = mid;
    }
    const auto& pA = m_lookupTable[lo];
    const auto& pB = m_lookupTable[hi];
    double denom = pB.raw - pA.raw;
    if (qFuzzyIsNull(denom)) return (pA.scaled + pB.scaled) / 2.0;
    double t = (raw - pA.raw) / denom;
    return pA.scaled + t * (pB.scaled - pA.scaled);
}

/** @brief 指数转换: y = a * exp(b * x) + c */
double DataScalerWidget::convertExponential(double raw) const
{
    return m_expA * qExp(m_expB * raw) + m_expC;
}

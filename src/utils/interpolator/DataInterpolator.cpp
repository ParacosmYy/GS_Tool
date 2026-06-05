/**
 * @file DataInterpolator.cpp
 * @brief 数据插值引擎实现 — 线性/样条/最近邻/Lagrange多项式
 */

#include "utils/interpolator/DataInterpolator.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DataInterpolator::DataInterpolator(QObject* parent)
    : QObject(parent)
    , m_method(InterpolationMethod::Linear)
    , m_errorSum(0.0)
{
}

/** @brief 设置插值方法 @param method 方法 */
void DataInterpolator::setMethod(InterpolationMethod method)
{
    m_method = method;
}

/** @brief 设置控制点 @param xPoints X坐标 @param yPoints Y坐标 */
void DataInterpolator::setControlPoints(
    const QVector<double>& xPoints, const QVector<double>& yPoints)
{
    m_xPoints = xPoints;
    m_yPoints = yPoints;
    ++m_stats.cacheMisses;

    /* 预计算三次样条系数 */
    if (m_method == InterpolationMethod::CubicSpline && m_xPoints.size() >= 3) {
        int n = m_xPoints.size() - 1;
        m_cubicCoeffs.resize(4 * n);
        QVector<double> h(n), alpha(n), l(n + 1), mu(n + 1), z(n + 1);

        for (int i = 0; i < n; ++i) {
            h[i] = m_xPoints[i + 1] - m_xPoints[i];
        }
        for (int i = 1; i < n; ++i) {
            alpha[i] = 3.0 / h[i] * (m_yPoints[i + 1] - m_yPoints[i])
                     - 3.0 / h[i - 1] * (m_yPoints[i] - m_yPoints[i - 1]);
        }

        l[0] = 1.0; mu[0] = 0.0; z[0] = 0.0;
        for (int i = 1; i < n; ++i) {
            l[i] = 2.0 * (m_xPoints[i + 1] - m_xPoints[i - 1]) - h[i - 1] * mu[i - 1];
            mu[i] = h[i] / l[i];
            z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
        }
        l[n] = 1.0; z[n] = 0.0;

        QVector<double> c(n + 1), b(n), d(n);
        for (int j = n - 1; j >= 0; --j) {
            c[j] = z[j] - mu[j] * c[j + 1];
            b[j] = (m_yPoints[j + 1] - m_yPoints[j]) / h[j]
                 - h[j] * (c[j + 1] + 2.0 * c[j]) / 3.0;
            d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
        }

        for (int i = 0; i < n; ++i) {
            m_cubicCoeffs[4 * i + 0] = m_yPoints[i];
            m_cubicCoeffs[4 * i + 1] = b[i];
            m_cubicCoeffs[4 * i + 2] = c[i];
            m_cubicCoeffs[4 * i + 3] = d[i];
        }
    }
}

/** @brief 在指定X处插值 @param x 目标X @return 插值Y */
double DataInterpolator::interpolate(double x)
{
    if (m_xPoints.isEmpty()) {
        return 0.0;
    }
    if (m_xPoints.size() == 1) {
        return m_yPoints[0];
    }

    double result = 0.0;
    switch (m_method) {
    case InterpolationMethod::Linear:
        result = interpolateLinear(x);
        break;
    case InterpolationMethod::CubicSpline:
        result = interpolateCubic(x);
        break;
    case InterpolationMethod::Nearest:
        result = interpolateNearest(x);
        break;
    case InterpolationMethod::Polynomial:
        result = interpolatePolynomial(x);
        break;
    }

    ++m_stats.totalInterpolations;
    return result;
}

/** @brief 批量插值 @param xValues X坐标列表 @return Y值列表 */
QVector<double> DataInterpolator::interpolateBatch(
    const QVector<double>& xValues)
{
    QVector<double> results;
    results.reserve(xValues.size());
    for (double x : xValues) {
        results.append(interpolate(x));
    }
    emit interpolationComplete(xValues.size());
    return results;
}

/** @brief 重置统计 */
void DataInterpolator::resetStatistics()
{
    m_stats = Stats{};
    m_errorSum = 0.0;
}

/** @brief 线性插值 @param x 目标X @return Y值 */
double DataInterpolator::interpolateLinear(double x) const
{
    int seg = findSegment(x);
    if (seg < 0) return m_yPoints.first();
    if (seg >= m_xPoints.size() - 1) return m_yPoints.last();

    double x0 = m_xPoints[seg], x1 = m_xPoints[seg + 1];
    double y0 = m_yPoints[seg], y1 = m_yPoints[seg + 1];
    double denom = x1 - x0;
    if (qFuzzyIsNull(denom)) return y0;
    double t = (x - x0) / denom;
    return y0 + t * (y1 - y0);
}

/** @brief 三次样条插值 @param x 目标X @return Y值 */
double DataInterpolator::interpolateCubic(double x) const
{
    if (m_cubicCoeffs.isEmpty()) {
        return interpolateLinear(x);
    }

    int seg = findSegment(x);
    if (seg < 0) seg = 0;
    if (seg >= m_xPoints.size() - 1) seg = m_xPoints.size() - 2;

    double dx = x - m_xPoints[seg];
    int idx = 4 * seg;
    double a = m_cubicCoeffs[idx];
    double b = m_cubicCoeffs[idx + 1];
    double c = m_cubicCoeffs[idx + 2];
    double d = m_cubicCoeffs[idx + 3];

    return a + b * dx + c * dx * dx + d * dx * dx * dx;
}

/** @brief 最近邻插值 @param x 目标X @return Y值 */
double DataInterpolator::interpolateNearest(double x) const
{
    int seg = findSegment(x);
    if (seg < 0) return m_yPoints.first();
    if (seg >= m_xPoints.size() - 1) return m_yPoints.last();

    double mid = (m_xPoints[seg] + m_xPoints[seg + 1]) / 2.0;
    return (x <= mid) ? m_yPoints[seg] : m_yPoints[seg + 1];
}

/** @brief Lagrange多项式插值 @param x 目标X @return Y值 */
double DataInterpolator::interpolatePolynomial(double x) const
{
    double result = 0.0;
    int n = m_xPoints.size();

    /* 限制多项式阶数避免数值不稳定 */
    int limit = qMin(n, 20);

    for (int i = 0; i < limit; ++i) {
        double term = m_yPoints[i];
        for (int j = 0; j < limit; ++j) {
            if (i != j) {
                double denom = m_xPoints[i] - m_xPoints[j];
                if (qFuzzyIsNull(denom)) {
                    term = 0.0;
                    break;
                }
                term *= (x - m_xPoints[j]) / denom;
            }
        }
        result += term;
    }
    return result;
}

/** @brief 查找X所在的段索引 @param x 目标X @return 段索引 */
int DataInterpolator::findSegment(double x) const
{
    if (m_xPoints.isEmpty()) return -1;
    if (x <= m_xPoints.first()) return 0;
    if (x >= m_xPoints.last()) return m_xPoints.size() - 1;

    int lo = 0, hi = m_xPoints.size() - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (m_xPoints[mid] <= x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

/**
 * @file ChebyshevPoly.cpp
 * @brief Chebyshev多项式逼近实现 — 系数求解/Clenshaw求值/导数/积分
 *
 * 使用Chebyshev-Gauss节点和离散余弦变换思想求解系数，
 * Clenshaw递推实现稳定求值，支持导数和定积分的解析计算。
 */

#include "utils/poly3/ChebyshevPoly.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
ChebyshevPoly::ChebyshevPoly(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ChebyshevPoly"));
}

// ── 拟合 ──

/**
 * @brief 从数据点拟合Chebyshev系数
 * @param x X坐标
 * @param y Y坐标
 * @param degree 多项式阶数
 * @return 拟合结果(系数+误差)
 *
 * 将x映射到[-1,1]，在Chebyshev-Gauss节点上计算系数。
 */
ChebyshevPoly::FitResult ChebyshevPoly::fitFromPoints(
    const QVector<double>& x, const QVector<double>& y, int degree)
{
    FitResult result;
    if (x.size() != y.size() || x.size() < 2 || degree < 0) {
        emit fitCompleted(result);
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    m_degree = qMin(degree, n - 1);

    /* 记录原始区间 */
    m_xMin = *std::min_element(x.begin(), x.end());
    m_xMax = *std::max_element(x.begin(), x.end());
    if (qFuzzyCompare(m_xMin, m_xMax)) m_xMax = m_xMin + 1.0;

    /* 映射到[-1,1] */
    QVector<double> t(n);
    for (int i = 0; i < n; ++i) {
        t[i] = mapToUnit(x[i]);
    }

    /* 计算Chebyshev系数 c_k = (2/N) * sum_j y_j * T_k(t_j) */
    m_coeffs.resize(m_degree + 1);
    for (int k = 0; k <= m_degree; ++k) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += y[j] * qCos(k * qAcos(qBound(-1.0, t[j], 1.0)));
        }
        m_coeffs[k] = sum * 2.0 / static_cast<double>(n);
    }
    m_coeffs[0] *= 0.5;  /* T0系数减半 */

    /* 计算逼近误差 */
    double maxErr = 0.0;
    double sumSq = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = y[i] - clenshaw(t[i]);
        maxErr = qMax(maxErr, qAbs(diff));
        sumSq += diff * diff;
    }

    result.coefficients = m_coeffs;
    result.maxError = maxErr;
    result.rmsError = qSqrt(sumSq / static_cast<double>(n));
    result.degree = m_degree;

    /* 更新统计 */
    ++m_stats.totalFits;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(result);
    return result;
}

/**
 * @brief 从函数对象拟合Chebyshev系数
 * @param func 被逼近函数
 * @param a 区间左端
 * @param b 区间右端
 * @param degree 多项式阶数
 * @return 拟合结果
 */
ChebyshevPoly::FitResult ChebyshevPoly::fitFromFunction(
    const std::function<double(double)>& func,
    double a, double b, int degree)
{
    /* 在Chebyshev-Gauss节点上采样 */
    int nNodes = qMax(degree + 1, 16);
    auto nodes = chebyshevNodes(nNodes, a, b);
    QVector<double> y(nNodes);
    for (int i = 0; i < nNodes; ++i) {
        y[i] = func(nodes[i]);
    }
    return fitFromPoints(nodes.toVector(), y, degree);
}

// ── 求值 ──

/**
 * @brief 使用Clenshaw递推求值
 * @param x 原始坐标
 * @return 函数值
 */
double ChebyshevPoly::evaluate(double x) const
{
    if (m_coeffs.isEmpty()) return 0.0;
    double t = mapToUnit(x);
    ++const_cast<ChebyshevPoly*>(this)->m_stats.totalEvaluations;
    return clenshaw(qBound(-1.0, t, 1.0));
}

/**
 * @brief 批量求值
 * @param xPoints 求值点
 * @return 函数值列表
 */
QVector<double> ChebyshevPoly::evaluateBatch(
    const QVector<double>& xPoints) const
{
    QVector<double> result;
    result.reserve(xPoints.size());
    for (double x : xPoints) {
        double t = mapToUnit(x);
        result.append(clenshaw(qBound(-1.0, t, 1.0)));
    }
    const_cast<ChebyshevPoly*>(this)->m_stats.totalEvaluations
        += xPoints.size();
    return result;
}

/**
 * @brief 计算导数
 * @param x 原始坐标
 * @return 导数值
 *
 * 利用Chebyshev导数递推: T'_n = n * U_{n-1}
 * 通过导数系数的Clenshaw递推实现。
 */
double ChebyshevPoly::derivative(double x) const
{
    if (m_coeffs.size() < 2) return 0.0;

    ++const_cast<ChebyshevPoly*>(this)->m_stats.totalDerivatives;

    auto dCoeffs = derivativeCoefficients();
    double t = qBound(-1.0, mapToUnit(x), 1.0);
    int n = dCoeffs.size();
    if (n == 0) return 0.0;

    /* Clenshaw递推对导数系数求值 */
    double y2 = 0.0, y1 = 0.0;
    for (int k = n - 1; k >= 1; --k) {
        double y0 = dCoeffs[k] + 2.0 * t * y1 - y2;
        y2 = y1;
        y1 = y0;
    }
    double val = dCoeffs[0] + t * y1 - y2;

    /* 链式法则: df/dx = (df/dt) * (dt/dx) */
    double scale = 2.0 / (m_xMax - m_xMin);
    return val * scale;
}

/**
 * @brief 计算定积分
 * @param a 积分下限
 * @param b 积分上限
 * @return 积分值
 *
 * 利用Chebyshev积分公式: integral T_k = ...
 * 使用数值Gauss-Chebyshev求积实现。
 */
double ChebyshevPoly::integrate(double a, double b) const
{
    if (m_coeffs.isEmpty()) return 0.0;

    ++const_cast<ChebyshevPoly*>(this)->m_stats.totalIntegrations;

    /* Gauss-Chebyshev求积 */
    int nPts = qMax(32, m_degree * 4);
    double sum = 0.0;
    double ha = (b - a) * 0.5;
    double hb = (b + a) * 0.5;

    for (int k = 0; k < nPts; ++k) {
        double theta = (static_cast<double>(k) + 0.5) * M_PI
            / static_cast<double>(nPts);
        double t = qCos(theta);
        double x = hb + ha * t;
        double w = ha * M_PI / static_cast<double>(nPts);
        sum += w * qSqrt(1.0 - t * t) * clenshaw(t);
    }

    return sum;
}

/**
 * @brief 计算Chebyshev-Gauss节点
 * @param n 节点数
 * @param a 区间左端
 * @param b 区间右端
 * @return 节点坐标
 */
QVector<double> ChebyshevPoly::chebyshevNodes(int n, double a, double b)
{
    QVector<double> nodes(n);
    double halfRange = (b - a) * 0.5;
    double center = (a + b) * 0.5;
    for (int k = 0; k < n; ++k) {
        double theta = (static_cast<double>(k) + 0.5) * M_PI
            / static_cast<double>(n);
        nodes[k] = center + halfRange * qCos(theta);
    }
    return nodes;
}

/**
 * @brief 计算Chebyshev-Lobatto节点(含端点)
 * @param n 节点数
 * @param a 区间左端
 * @param b 区间右端
 * @return 节点坐标
 */
QVector<double> ChebyshevPoly::lobattoNodes(int n, double a, double b)
{
    QVector<double> nodes(n);
    double halfRange = (b - a) * 0.5;
    double center = (a + b) * 0.5;
    for (int k = 0; k < n; ++k) {
        double theta = M_PI * static_cast<double>(k)
            / static_cast<double>(n - 1);
        nodes[k] = center + halfRange * qCos(theta);
    }
    return nodes;
}

// ── 私有方法 ──

/** @brief 映射到[-1,1] */
double ChebyshevPoly::mapToUnit(double x) const
{
    if (qFuzzyCompare(m_xMax, m_xMin)) return 0.0;
    return 2.0 * (x - m_xMin) / (m_xMax - m_xMin) - 1.0;
}

/** @brief 从[-1,1]映射回原始区间 */
double ChebyshevPoly::mapFromUnit(double t) const
{
    return m_xMin + (t + 1.0) * 0.5 * (m_xMax - m_xMin);
}

/**
 * @brief Clenshaw递推求值核心
 * @param t 参数[-1,1]
 * @return 多项式值
 */
double ChebyshevPoly::clenshaw(double t) const
{
    int n = m_coeffs.size();
    if (n == 0) return 0.0;
    if (n == 1) return m_coeffs[0];

    double b2 = 0.0;
    double b1 = m_coeffs[n - 1];
    for (int k = n - 2; k >= 1; --k) {
        double b0 = m_coeffs[k] + 2.0 * t * b1 - b2;
        b2 = b1;
        b1 = b0;
    }
    return m_coeffs[0] + t * b1 - b2;
}

/**
 * @brief 计算导数系数
 * @return 导数的Chebyshev系数
 */
QVector<double> ChebyshevPoly::derivativeCoefficients() const
{
    int n = m_coeffs.size();
    if (n < 2) return {0.0};

    QVector<double> dc(n - 1, 0.0);
    /* 递推: d_k = 2*(k+1)*c_{k+1} + d_{k+2} */
    dc[n - 2] = 2.0 * static_cast<double>(n - 1) * m_coeffs[n - 1];
    for (int k = n - 3; k >= 0; --k) {
        dc[k] = 2.0 * static_cast<double>(k + 1) * m_coeffs[k + 1]
            + ((k + 3 < n - 1) ? dc[k + 2] : 0.0);
    }
    return dc;
}

/** @brief 重置统计 */
void ChebyshevPoly::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

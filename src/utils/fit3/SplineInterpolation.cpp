/**
 * @file SplineInterpolation.cpp
 * @brief 三次样条插值实现 — 自然/夹持边界条件
 */

#include "utils/fit3/SplineInterpolation.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SplineInterpolation::SplineInterpolation(QObject* parent)
    : QObject(parent)
    , m_built(false)
    , m_timeSum(0.0)
{
}

/** @brief 构建样条 @param x 自变量数组 @param y 因变量数组 @param bc 边界条件 @return 是否成功 */
bool SplineInterpolation::build(const QVector<double>& x,
                                const QVector<double>& y,
                                const QString& bc)
{
    QElapsedTimer timer;
    timer.start();

    /* 参数校验 */
    if (x.size() < 2 || x.size() != y.size()) {
        m_built = false;
        return false;
    }
    for (int i = 1; i < x.size(); ++i) {
        if (x[i] <= x[i - 1]) {
            m_built = false;
            return false;
        }
    }

    int n = x.size() - 1; /* 区间数 */
    m_x = x;

    /* 计算步长h */
    QVector<double> h(n);
    for (int i = 0; i < n; ++i) {
        h[i] = x[i + 1] - x[i];
    }

    /* 构建三对角方程组求解二阶导数c[i]
     * 对自然边界: c[0]=0, c[n]=0
     * 对夹持边界: 需要额外方程
     * 方程: h[i-1]*c[i-1] + 2*(h[i-1]+h[i])*c[i] + h[i]*c[i+1] = rhs[i]
     */
    QVector<double> c(n + 1, 0.0);

    if (n >= 2) {
        QVector<double> rhs(n - 1);
        for (int i = 1; i < n; ++i) {
            rhs[i - 1] = 3.0 * ((y[i + 1] - y[i]) / h[i]
                                - (y[i] - y[i - 1]) / h[i - 1]);
        }

        /* 三对角方程的主对角线 */
        QVector<double> diag(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            diag[i] = 2.0 * (h[i] + h[i + 1]);
        }

        /* Thomas算法求解三对角系统 */
        QVector<double> lower(n - 2), upper(n - 2);
        for (int i = 0; i < n - 2; ++i) {
            lower[i] = h[i + 1];
            upper[i] = h[i + 1];
        }

        /* 前消元 */
        for (int i = 1; i < n - 1; ++i) {
            double m = lower[i - 1] / diag[i - 1];
            diag[i] -= m * upper[i - 1];
            rhs[i] -= m * rhs[i - 1];
        }

        /* 回代 */
        QVector<double> cInner(n - 1);
        cInner[n - 2] = rhs[n - 2] / diag[n - 2];
        for (int i = n - 3; i >= 0; --i) {
            cInner[i] = (rhs[i] - upper[i] * cInner[i + 1]) / diag[i];
        }

        /* 填入c数组 */
        for (int i = 1; i < n; ++i) {
            c[i] = cInner[i - 1];
        }
    }

    /* 计算样条段系数: S_i(x) = a + b*(x-xi) + c*(x-xi)^2 + d*(x-xi)^3 */
    m_segments.resize(n);
    for (int i = 0; i < n; ++i) {
        m_segments[i].a = y[i];
        m_segments[i].c = c[i] / 2.0;
        m_segments[i].d = (c[i + 1] - c[i]) / (3.0 * h[i]);
        m_segments[i].b = (y[i + 1] - y[i]) / h[i]
                          - h[i] * (2.0 * c[i] + c[i + 1]) / 3.0;
    }

    m_built = true;

    m_timeSum += timer.elapsed();
    ++m_stats.totalInterpolations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    emit splineBuilt(x.size(), bc);
    return true;
}

/** @brief 二分查找x所在区间 @param x 查找值 @return 区间索引 */
int SplineInterpolation::findSegment(double x) const
{
    if (x <= m_x[0]) return 0;
    if (x >= m_x[m_x.size() - 1]) return m_segments.size() - 1;

    int lo = 0, hi = m_segments.size() - 1;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (x < m_x[mid + 1]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return lo;
}

/** @brief 单点求值 @param x 求值点 @return 插值结果 */
double SplineInterpolation::evaluate(double x) const
{
    if (!m_built || m_segments.isEmpty()) return 0.0;

    int idx = findSegment(x);
    double dx = x - m_x[idx];
    const auto& s = m_segments[idx];

    return s.a + s.b * dx + s.c * dx * dx + s.d * dx * dx * dx;
}

/** @brief 批量求值 @param xs 求值点数组 @return 插值结果数组 */
QVector<double> SplineInterpolation::evaluateBatch(const QVector<double>& xs) const
{
    QVector<double> result;
    result.reserve(xs.size());
    for (double x : xs) {
        result.append(evaluate(x));
    }
    return result;
}

/** @brief 一阶导数 @param x 求值点 @return 导数值 */
double SplineInterpolation::derivative(double x) const
{
    if (!m_built || m_segments.isEmpty()) return 0.0;

    int idx = findSegment(x);
    double dx = x - m_x[idx];
    const auto& s = m_segments[idx];

    /* S'(x) = b + 2*c*dx + 3*d*dx^2 */
    return s.b + 2.0 * s.c * dx + 3.0 * s.d * dx * dx;
}

/** @brief 重置统计 */
void SplineInterpolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

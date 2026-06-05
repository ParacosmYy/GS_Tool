/**
 * @file CubicSpline2.cpp
 * @brief 自然三次样条插值实现 — 三对角方程求解 + 导数评估
 */

#include "utils/interp9/CubicSpline2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CubicSpline2::CubicSpline2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 从节点数据构建自然三次样条 @param xKnots x坐标节点(必须递增) @param yKnots y坐标节点值 @return 是否构建成功 */
bool CubicSpline2::build(const QVector<double>& xKnots,
                          const QVector<double>& yKnots)
{
    QElapsedTimer timer;
    timer.start();

    clear();

    int n = xKnots.size();
    if (n < 2 || yKnots.size() != n) return false;

    /* 检查x严格递增 */
    for (int i = 1; i < n; ++i) {
        if (xKnots[i] <= xKnots[i - 1]) return false;
    }

    m_xKnots = xKnots;
    m_yKnots = yKnots;

    /* 自然三次样条: 求解三对角方程组得到二阶导数M_i */
    /* 方程: h_{i-1}*M_{i-1} + 2*(h_{i-1}+h_i)*M_i + h_i*M_{i+1} = 6*(rhs_i) */
    int m = n - 2; /* 内部节点数(不含端点) */
    if (m <= 0) {
        /* 两点线性插值 */
        m_coeffs.resize(1);
        m_coeffs[0].a = yKnots[0];
        m_coeffs[0].b = (yKnots[1] - yKnots[0]) / (xKnots[1] - xKnots[0]);
        m_coeffs[0].c = 0.0;
        m_coeffs[0].d = 0.0;
        m_built = true;
        return true;
    }

    /* 计算步长h */
    QVector<double> h(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        h[i] = xKnots[i + 1] - xKnots[i];
    }

    /* 构造三对角方程组 */
    QVector<double> lower(m, 0.0);     /* 下对角线 */
    QVector<double> mainDiag(m, 0.0);  /* 主对角线 */
    QVector<double> upper(m, 0.0);     /* 上对角线 */
    QVector<double> rhs(m, 0.0);       /* 右端项 */

    for (int i = 0; i < m; ++i) {
        int k = i + 1; /* 实际节点索引 */
        mainDiag[i] = 2.0 * (h[k - 1] + h[k]);

        if (i > 0) lower[i] = h[k - 1];
        if (i < m - 1) upper[i] = h[k];

        rhs[i] = 6.0 * ((yKnots[k + 1] - yKnots[k]) / h[k]
                         - (yKnots[k] - yKnots[k - 1]) / h[k - 1]);
    }

    /* Thomas算法求解 */
    QVector<double> M_inner = thomasSolve(lower, mainDiag, upper, rhs);

    /* 完整M数组: 自然边界 M_0 = M_{n-1} = 0 */
    QVector<double> M(n, 0.0);
    for (int i = 0; i < m; ++i) {
        M[i + 1] = M_inner[i];
    }

    /* 计算每段样条系数 */
    m_coeffs.resize(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        m_coeffs[i].a = yKnots[i];
        m_coeffs[i].b = (yKnots[i + 1] - yKnots[i]) / h[i]
                         - h[i] * (2.0 * M[i] + M[i + 1]) / 6.0;
        m_coeffs[i].c = M[i] / 2.0;
        m_coeffs[i].d = (M[i + 1] - M[i]) / (6.0 * h[i]);
    }

    m_built = true;

    /* 更新统计 */
    m_stats.totalSplinesBuilt++;
    m_stats.totalKnotPoints += n;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSplinesBuilt);

    emit splineBuilt(n);
    return true;
}

/** @brief 评估样条值 @param x 评估点 @return 插值结果 */
double CubicSpline2::evaluate(double x) const
{
    if (!m_built) return qQNaN();

    int seg = findSegment(x);
    if (seg < 0) seg = 0;
    if (seg >= m_coeffs.size()) seg = m_coeffs.size() - 1;

    double dx = x - m_xKnots[seg];
    const auto& c = m_coeffs[seg];
    return c.a + c.b * dx + c.c * dx * dx + c.d * dx * dx * dx;
}

/** @brief 批量评估样条值 @param xPoints 评估点数组 @return 插值结果数组 */
QVector<double> CubicSpline2::evaluateBatch(const QVector<double>& xPoints) const
{
    QVector<double> result(xPoints.size());
    for (int i = 0; i < xPoints.size(); ++i) {
        result[i] = evaluate(xPoints[i]);
    }

    const_cast<CubicSpline2*>(this)->m_stats.totalPointsEvaluated += xPoints.size();
    emit const_cast<CubicSpline2*>(this)->evaluationComplete(xPoints.size());
    return result;
}

/** @brief 评估一阶导数 @param x 评估点 @return 导数值 */
double CubicSpline2::evaluateDerivative(double x) const
{
    if (!m_built) return qQNaN();

    int seg = findSegment(x);
    if (seg < 0) seg = 0;
    if (seg >= m_coeffs.size()) seg = m_coeffs.size() - 1;

    double dx = x - m_xKnots[seg];
    const auto& c = m_coeffs[seg];
    return c.b + 2.0 * c.c * dx + 3.0 * c.d * dx * dx;
}

/** @brief 评估二阶导数 @param x 评估点 @return 二阶导数值 */
double CubicSpline2::evaluateSecondDerivative(double x) const
{
    if (!m_built) return qQNaN();

    int seg = findSegment(x);
    if (seg < 0) seg = 0;
    if (seg >= m_coeffs.size()) seg = m_coeffs.size() - 1;

    double dx = x - m_xKnots[seg];
    const auto& c = m_coeffs[seg];
    return 2.0 * c.c + 6.0 * c.d * dx;
}

/** @brief 计算样条在[a,b]上的定积分 @param a 积分下限 @param b 积分上限 @return 积分值 */
double CubicSpline2::integrate(double a, double b) const
{
    if (!m_built) return qQNaN();

    int segA = findSegment(a);
    int segB = findSegment(b);
    if (segA < 0) segA = 0;
    if (segB >= m_coeffs.size()) segB = m_coeffs.size() - 1;

    double total = 0.0;
    for (int i = segA; i <= segB; ++i) {
        double x0 = qMax(a, m_xKnots[i]);
        double x1 = qMin(b, m_xKnots[i + 1]);
        if (x1 <= x0) continue;

        double d0 = x0 - m_xKnots[i];
        double d1 = x1 - m_xKnots[i];
        const auto& c = m_coeffs[i];

        /* ∫(a+b*dx+c*dx²+d*dx³)dx = a*dx + b*dx²/2 + c*dx³/3 + d*dx⁴/4 */
        auto integ = [&](double dx) -> double {
            return c.a * dx + c.b * dx * dx / 2.0
                   + c.c * dx * dx * dx / 3.0
                   + c.d * dx * dx * dx * dx / 4.0;
        };

        total += integ(d1) - integ(d0);
    }
    return total;
}

/** @brief 获取指定段的系数 @param index 段索引 @return 系数 */
CubicSpline2::SplineCoeffs CubicSpline2::segmentCoeffs(int index) const
{
    if (index < 0 || index >= m_coeffs.size()) return {};
    return m_coeffs[index];
}

/** @brief 获取所有段系数 @return 系数数组 */
QVector<CubicSpline2::SplineCoeffs> CubicSpline2::allCoeffs() const
{
    return m_coeffs;
}

/** @brief 获取节点数 @return 节点数 */
int CubicSpline2::knotCount() const
{
    return m_xKnots.size();
}

/** @brief 样条是否已构建 @return 是否可用 */
bool CubicSpline2::isBuilt() const
{
    return m_built;
}

/** @brief 清除样条数据 */
void CubicSpline2::clear()
{
    m_xKnots.clear();
    m_yKnots.clear();
    m_coeffs.clear();
    m_built = false;
}

/** @brief 重置统计 */
void CubicSpline2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Thomas算法求解三对角方程组 */
QVector<double> CubicSpline2::thomasSolve(const QVector<double>& lower,
                                           const QVector<double>& mainDiag,
                                           const QVector<double>& upper,
                                           const QVector<double>& rhs) const
{
    int n = rhs.size();
    if (n == 0) return {};

    QVector<double> c(upper);
    QVector<double> d(rhs);

    /* 前向消元 */
    for (int i = 1; i < n; ++i) {
        double m = lower[i] / mainDiag[i - 1];
        mainDiag[i] -= m * upper[i - 1];
        d[i] -= m * d[i - 1];
    }

    /* 回代 */
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / mainDiag[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = (d[i] - upper[i] * x[i + 1]) / mainDiag[i];
    }
    return x;
}

/** @brief 二分查找x所在的段索引 */
int CubicSpline2::findSegment(double x) const
{
    if (m_xKnots.size() < 2) return -1;

    /* 外推: 返回最近段 */
    if (x <= m_xKnots[0]) return 0;
    if (x >= m_xKnots[m_xKnots.size() - 1]) return m_coeffs.size() - 1;

    /* 二分查找 */
    int lo = 0, hi = m_xKnots.size() - 2;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (x < m_xKnots[mid + 1]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    return lo;
}

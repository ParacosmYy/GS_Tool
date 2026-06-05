/**
 * @file CubicInterpolator.cpp
 * @brief 三次样条插值实现
 */

#include "utils/interp2/CubicInterpolator.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
CubicInterpolator::CubicInterpolator(QObject* parent)
    : QObject(parent)
    , m_built(false)
    , m_timeSum(0.0)
{
}

/** @brief 自然三次样条 */
void CubicInterpolator::buildNatural(const QVector<double>& x,
                                      const QVector<double>& y)
{
    int n = x.size() - 1;
    if (n < 1) return;

    m_x = x;
    m_a = y;

    /* 求解三对角方程获得c系数 */
    QVector<double> h(n);
    for (int i = 0; i < n; ++i)
        h[i] = x[i + 1] - x[i];

    /* 构建右端 */
    QVector<double> rhs(n + 1, 0.0);
    for (int i = 1; i < n; ++i)
        rhs[i] = 3.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);

    /* 三对角系统(自然边界c[0]=c[n]=0) */
    QVector<double> diag(n + 1, 2.0);
    QVector<double> upper(n, 0.0);
    QVector<double> lower(n, 0.0);

    for (int i = 1; i < n; ++i) {
        lower[i] = h[i - 1];
        upper[i - 1] = h[i];
    }

    m_c = solveTridiagonal(diag, upper, lower, rhs);

    /* 计算b和d */
    m_b.resize(n);
    m_d.resize(n);
    for (int i = 0; i < n; ++i) {
        m_b[i] = (y[i + 1] - y[i]) / h[i] - h[i] * (2.0 * m_c[i] + m_c[i + 1]) / 3.0;
        m_d[i] = (m_c[i + 1] - m_c[i]) / (3.0 * h[i]);
    }

    m_built = true;
}

/** @brief 夹紧三次样条 */
void CubicInterpolator::buildClamped(const QVector<double>& x,
                                      const QVector<double>& y,
                                      double dLeft, double dRight)
{
    int n = x.size() - 1;
    if (n < 1) return;

    m_x = x;
    m_a = y;

    QVector<double> h(n);
    for (int i = 0; i < n; ++i)
        h[i] = x[i + 1] - x[i];

    QVector<double> rhs(n + 1, 0.0);
    rhs[0] = 3.0 * ((y[1] - y[0]) / h[0] - dLeft);
    for (int i = 1; i < n; ++i)
        rhs[i] = 3.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);
    rhs[n] = 3.0 * (dRight - (y[n] - y[n - 1]) / h[n - 1]);

    QVector<double> diag(n + 1, 2.0);
    QVector<double> upper(n, 0.0);
    QVector<double> lower(n, 0.0);

    diag[0] = 2.0 * h[0];
    diag[n] = 2.0 * h[n - 1];
    upper[0] = h[0];
    lower[n - 1] = h[n - 1];

    for (int i = 1; i < n; ++i) {
        lower[i] = h[i - 1];
        upper[i - 1] = h[i];
    }

    m_c = solveTridiagonal(diag, upper, lower, rhs);

    m_b.resize(n);
    m_d.resize(n);
    for (int i = 0; i < n; ++i) {
        m_b[i] = (y[i + 1] - y[i]) / h[i] - h[i] * (2.0 * m_c[i] + m_c[i + 1]) / 3.0;
        m_d[i] = (m_c[i + 1] - m_c[i]) / (3.0 * h[i]);
    }

    m_built = true;
}

/** @brief 求值 */
double CubicInterpolator::evaluate(double x) const
{
    if (!m_built || m_x.isEmpty()) return 0.0;

    int i = findSegment(x);
    if (i < 0) i = 0;
    if (i >= m_b.size()) i = m_b.size() - 1;

    double dx = x - m_x[i];
    return m_a[i] + m_b[i] * dx + m_c[i] * dx * dx + m_d[i] * dx * dx * dx;
}

/** @brief 批量求值 */
QVector<double> CubicInterpolator::evaluateBatch(
    const QVector<double>& xPoints) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    result.reserve(xPoints.size());
    for (double x : xPoints)
        result.append(evaluate(x));

    double elapsed = static_cast<double>(timer.elapsed());
    const_cast<CubicInterpolator*>(this)->m_timeSum += elapsed;
    auto& s = const_cast<CubicInterpolator*>(this)->m_stats;
    ++s.totalInterpolations;
    s.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(s.totalInterpolations);

    emit const_cast<CubicInterpolator*>(this)->interpolationCompleted(xPoints.size());
    return result;
}

/** @brief 重置统计 */
void CubicInterpolator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 三对角方程Thomas算法 */
QVector<double> CubicInterpolator::solveTridiagonal(
    const QVector<double>& diag,
    const QVector<double>& upper,
    const QVector<double>& lower,
    const QVector<double>& rhs)
{
    int n = diag.size();
    QVector<double> cPrime(n);
    QVector<double> dPrime(n);
    QVector<double> x(n);

    cPrime[0] = (n > 1) ? upper[0] / diag[0] : 0.0;
    dPrime[0] = rhs[0] / diag[0];

    for (int i = 1; i < n; ++i) {
        double m = diag[i] - lower[i] * cPrime[i - 1];
        if (qAbs(m) < 1e-15) m = 1e-15;
        cPrime[i] = (i < n - 1) ? upper[i] / m : 0.0;
        dPrime[i] = (rhs[i] - lower[i] * dPrime[i - 1]) / m;
    }

    x[n - 1] = dPrime[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dPrime[i] - cPrime[i] * x[i + 1];

    return x;
}

/** @brief 查找区间 */
int CubicInterpolator::findSegment(double x) const
{
    int n = m_x.size();
    if (x <= m_x[0]) return 0;
    if (x >= m_x[n - 1]) return n - 2;

    int lo = 0, hi = n - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (x < m_x[mid]) hi = mid;
        else lo = mid;
    }
    return lo;
}

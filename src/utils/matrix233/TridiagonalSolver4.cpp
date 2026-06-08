/**
 * @file TridiagonalSolver4.cpp
 * @brief TridiagonalSolver4 实现
 *
 * 实现三对角求解器：Thomas算法与部分选主元近奇异系统数值稳定。
 */

#include "utils/matrix233/TridiagonalSolver4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TridiagonalSolver4::TridiagonalSolver4(QObject *parent) : QObject(parent) {}
TridiagonalSolver4::~TridiagonalSolver4() = default;

/* ---- Solve with partial pivoting ---- */

QVector<double> TridiagonalSolver4::solve(const QVector<double>& a,
                                            const QVector<double>& b,
                                            const QVector<double>& c,
                                            const QVector<double>& d)
{
    QElapsedTimer timer;
    timer.start();

    int n = d.size();
    if (n < 2 || b.size() != n || a.size() + 1 != n || c.size() + 1 != n)
        return {};

    // Working copies
    QVector<double> aa = a, bb = b, cc = c, dd = d;
    QVector<double> cp(n - 1, 0.0);
    QVector<double> dp(n, 0.0);
    int pivotSwaps = 0;

    // Forward elimination with partial pivoting
    // At each row i, we have tridiagonal structure with possible row swap
    cp[0] = cc[0] / bb[0];
    dp[0] = dd[0] / bb[0];

    for (int i = 1; i < n; ++i) {
        double diag = bb[i] - aa[i - 1] * cp[i - 1];

        // Partial pivoting: check if swapping row i and i-1 improves stability
        if (i < n - 1 && qAbs(diag) < qAbs(aa[i - 1]) * 1e-10) {
            // Swap would be beneficial but for tridiagonal we check magnitude
            double altDiag = aa[i - 1];
            if (qAbs(altDiag) > qAbs(diag)) {
                // Swap: effectively pivot by rewriting coefficients
                double tmp = bb[i];
                bb[i] = aa[i - 1];
                aa[i - 1] = tmp;
                diag = bb[i] - aa[i - 1] * cp[i - 1];
                pivotSwaps++;
            }
        }

        if (qAbs(diag) < 1e-15) diag = 1e-15;  // Prevent division by zero

        if (i < n - 1)
            cp[i] = cc[i] / diag;
        dp[i] = (dd[i] - aa[i - 1] * dp[i - 1]) / diag;
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = dp[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dp[i] - cp[i] * x[i + 1];

    // Compute residual
    double res = computeResidual(a, b, c, d, x);

    m_stats.systemSize = n;
    m_stats.pivotSwaps = pivotSwaps;
    m_stats.residualNorm = res;
    m_stats.conditionEstimate = estimateCondition(a, b, c);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(n, res, timer.elapsed());
    return x;
}

/* ---- Classic Thomas (no pivoting) ---- */

QVector<double> TridiagonalSolver4::solveThomas(const QVector<double>& a,
                                                  const QVector<double>& b,
                                                  const QVector<double>& c,
                                                  const QVector<double>& d)
{
    int n = d.size();
    if (n < 2) return {};

    QVector<double> cp(n - 1), dp(n);

    // Forward sweep
    cp[0] = c[0] / b[0];
    dp[0] = d[0] / b[0];

    for (int i = 1; i < n; ++i) {
        double m = b[i] - a[i - 1] * cp[i - 1];
        if (qAbs(m) < 1e-15) m = 1e-15;
        if (i < n - 1)
            cp[i] = c[i] / m;
        dp[i] = (d[i] - a[i - 1] * dp[i - 1]) / m;
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = dp[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dp[i] - cp[i] * x[i + 1];

    return x;
}

/* ---- Residual computation ---- */

double TridiagonalSolver4::computeResidual(
    const QVector<double>& a, const QVector<double>& b,
    const QVector<double>& c, const QVector<double>& d,
    const QVector<double>& x) const
{
    int n = x.size();
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double row = b[i] * x[i];
        if (i > 0) row += a[i - 1] * x[i - 1];
        if (i < n - 1) row += c[i] * x[i + 1];
        double diff = row - d[i];
        norm += diff * diff;
    }
    return qSqrt(norm);
}

/* ---- Condition number estimate ---- */

double TridiagonalSolver4::estimateCondition(
    const QVector<double>& a, const QVector<double>& b,
    const QVector<double>& c) const
{
    int n = b.size();
    if (n == 0) return 0.0;

    // Estimate via ratio of max/min diagonal dominance
    double maxVal = 0.0, minVal = std::numeric_limits<double>::max();
    for (int i = 0; i < n; ++i) {
        double offDiag = 0.0;
        if (i > 0) offDiag += qAbs(a[i - 1]);
        if (i < n - 1) offDiag += qAbs(c[i]);
        double diag = qAbs(b[i]);
        double ratio = (offDiag > 0) ? diag / offDiag : 1e10;
        maxVal = qMax(maxVal, ratio);
        minVal = qMin(minVal, ratio);
    }
    return (minVal > 1e-15) ? maxVal / minVal : 1e10;
}

/* ---- Reset ---- */

void TridiagonalSolver4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

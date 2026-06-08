/**
 * @file ThomasAlgorithm4.cpp
 * @brief ThomasAlgorithm4 实现
 *
 * 实现周期三对角Thomas算法：Sherman-Morrison辅助向量、环绕修正。
 */

#include "utils/matrix228/ThomasAlgorithm4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm4::ThomasAlgorithm4(QObject *parent) : QObject(parent) {}
ThomasAlgorithm4::~ThomasAlgorithm4() = default;

/* ---- Forward sweep ---- */

void ThomasAlgorithm4::forwardSweep(QVector<double>& bMod, QVector<double>& dMod,
                                      const QVector<double>& a, const QVector<double>& b,
                                      const QVector<double>& c, const QVector<double>& d) const
{
    int n = b.size();
    bMod[0] = b[0];
    dMod[0] = d[0];

    for (int i = 1; i < n; ++i) {
        double m = a[i] / bMod[i - 1];
        bMod[i] = b[i] - m * c[i - 1];
        dMod[i] = d[i] - m * dMod[i - 1];
    }
}

/* ---- Back substitution ---- */

QVector<double> ThomasAlgorithm4::backSubstitute(const QVector<double>& bMod,
                                                   const QVector<double>& c,
                                                   const QVector<double>& dMod) const
{
    int n = bMod.size();
    QVector<double> x(n);
    x[n - 1] = dMod[n - 1] / bMod[n - 1];

    for (int i = n - 2; i >= 0; --i)
        x[i] = (dMod[i] - c[i] * x[i + 1]) / bMod[i];

    return x;
}

/* ---- Standard Thomas solve ---- */

QVector<double> ThomasAlgorithm4::solve(const QVector<double>& a,
                                          const QVector<double>& b,
                                          const QVector<double>& c,
                                          const QVector<double>& d)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return QVector<double>();

    QVector<double> bMod(n), dMod(n);
    forwardSweep(bMod, dMod, a, b, c, d);
    QVector<double> x = backSubstitute(bMod, c, dMod);

    m_stats.systemSize = n;
    m_stats.isPeriodic = false;
    m_stats.residualNorm = residualNorm(a, b, c, d, x);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(n, false, m_stats.residualNorm, timer.elapsed());
    return x;
}

/* ---- Periodic Thomas solve (Sherman-Morrison) ---- */

QVector<double> ThomasAlgorithm4::solvePeriodic(const QVector<double>& a,
                                                   const QVector<double>& b,
                                                   const QVector<double>& c,
                                                   const QVector<double>& d)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n < 3) return solve(a, b, c, d);

    // The periodic system has corner elements: a[0] wraps to c[n-1], c[n-1] wraps to a[0]
    // Decompose using Sherman-Morrison: A = T + u*v^T
    // where T is standard tridiagonal, u*v^T accounts for periodicity

    // Build modified RHS vectors for two auxiliary systems
    double alpha = -b[0];       // Corner correction factor
    double beta = -b[n - 1];    // Corner correction factor

    // Auxiliary vector u: gamma at position 0, alpha at position n-1
    double gamma = b[0];
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = alpha;

    // Auxiliary vector v: 1 at position 0, c[n-1]/gamma at position n-1
    QVector<double> v(n, 0.0);
    v[0] = 1.0;
    v[n - 1] = a[0] / gamma;

    // Modified tridiagonal system (T): b[0] -= gamma, b[n-1] -= alpha
    QVector<double> bMod(n), dMod(n);
    for (int i = 0; i < n; ++i) {
        bMod[i] = b[i];
        dMod[i] = d[i];
    }
    bMod[0] -= gamma;
    bMod[n - 1] -= alpha;

    // Solve T * y = d
    QVector<double> bSweep(n), dSweep(n);
    forwardSweep(bSweep, dSweep, a, bMod, c, d);
    QVector<double> y = backSubstitute(bSweep, c, dSweep);

    // Solve T * z = u
    QVector<double> zSweep(n);
    forwardSweep(bSweep, zSweep, a, bMod, c, u);
    QVector<double> z = backSubstitute(bSweep, c, zSweep);

    // Sherman-Morrison correction: x = y - (v^T * y / (1 + v^T * z)) * z
    double vDotY = 0.0;
    double vDotZ = 0.0;
    for (int i = 0; i < n; ++i) {
        vDotY += v[i] * y[i];
        vDotZ += v[i] * z[i];
    }

    double factor = vDotY / (1.0 + vDotZ);
    QVector<double> x(n);
    for (int i = 0; i < n; ++i)
        x[i] = y[i] - factor * z[i];

    m_stats.systemSize = n;
    m_stats.isPeriodic = true;
    m_stats.residualNorm = residualNorm(a, b, c, d, x);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(n, true, m_stats.residualNorm, timer.elapsed());
    return x;
}

/* ---- Residual norm ---- */

double ThomasAlgorithm4::residualNorm(const QVector<double>& a,
                                        const QVector<double>& b,
                                        const QVector<double>& c,
                                        const QVector<double>& d,
                                        const QVector<double>& x) const
{
    int n = b.size();
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double res = b[i] * x[i] - d[i];
        if (i > 0) res += a[i] * x[i - 1];
        if (i < n - 1) res += c[i] * x[i + 1];
        norm += res * res;
    }
    return qSqrt(norm);
}

/* ---- Reset ---- */

void ThomasAlgorithm4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

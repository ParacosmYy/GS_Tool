/**
 * @file ThomasAlgorithm4.cpp
 * @brief ThomasAlgorithm4 实现
 *
 * 实现Thomas算法：部分主元消元与向量化前后向代入。
 */

#include "utils/matrix242/ThomasAlgorithm4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm4::ThomasAlgorithm4(QObject *parent) : QObject(parent) {}
ThomasAlgorithm4::~ThomasAlgorithm4() = default;

/* ---- Forward elimination with partial pivoting ---- */

void ThomasAlgorithm4::forwardElimination(QVector<double>& a, QVector<double>& b,
                                           QVector<double>& c, QVector<double>& d,
                                           int& pivotSwaps) const
{
    int n = b.size();
    pivotSwaps = 0;

    for (int i = 1; i < n; ++i) {
        // Partial pivoting: compare |a[i]| with |b[i-1]|
        if (qAbs(a[i]) > qAbs(b[i - 1])) {
            // Swap rows i and i-1
            std::swap(a[i], b[i - 1]);   // a[i] becomes new diagonal, b[i-1] becomes new sub-diag
            std::swap(b[i], c[i - 1]);   // diagonal and super-diagonal swap
            std::swap(d[i], d[i - 1]);
            pivotSwaps++;

            // Now eliminate with the pivoted row
            if (qAbs(b[i - 1]) < 1e-15) continue;
            double m = a[i] / b[i - 1];
            b[i] -= m * c[i - 1];
            d[i] -= m * d[i - 1];
            a[i] = 0.0;
        } else {
            if (qAbs(b[i - 1]) < 1e-15) continue;
            double m = a[i] / b[i - 1];
            b[i] -= m * c[i - 1];
            d[i] -= m * d[i - 1];
            a[i] = 0.0;
        }
    }
}

/* ---- Vectorized backward substitution ---- */

QVector<double> ThomasAlgorithm4::backwardSubstitution(const QVector<double>& b,
                                                        const QVector<double>& c,
                                                        const QVector<double>& d) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);

    // Back-substitute from last to first
    x[n - 1] = d[n - 1] / (qAbs(b[n - 1]) > 1e-15 ? b[n - 1] : 1e-15);
    for (int i = n - 2; i >= 0; --i) {
        double denom = qAbs(b[i]) > 1e-15 ? b[i] : 1e-15;
        x[i] = (d[i] - c[i] * x[i + 1]) / denom;
    }
    return x;
}

/* ---- Solve ---- */

QVector<double> ThomasAlgorithm4::solve(const TridiagonalSystem& system)
{
    int n = system.main.size();
    if (n < 2) return {};

    QElapsedTimer timer;
    timer.start();

    // Copy system for in-place elimination
    QVector<double> a = system.lower;
    QVector<double> b = system.main;
    QVector<double> c = system.upper;
    QVector<double> d = system.rhs;

    int pivotSwaps = 0;
    forwardElimination(a, b, c, d, pivotSwaps);
    QVector<double> x = backwardSubstitution(b, c, d);

    m_stats.systemSize = n;
    m_stats.numSolves++;
    m_stats.numPivotSwaps += pivotSwaps;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(n, pivotSwaps, timer.elapsed());
    return x;
}

/* ---- Solve in-place ---- */

QVector<double> ThomasAlgorithm4::solveInPlace(TridiagonalSystem& system)
{
    int n = system.main.size();
    if (n < 2) return {};

    QElapsedTimer timer;
    timer.start();

    int pivotSwaps = 0;
    forwardElimination(system.lower, system.main, system.upper, system.rhs, pivotSwaps);
    QVector<double> x = backwardSubstitution(system.main, system.upper, system.rhs);

    m_stats.systemSize = n;
    m_stats.numSolves++;
    m_stats.numPivotSwaps += pivotSwaps;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(n, pivotSwaps, timer.elapsed());
    return x;
}

/* ---- Batch solve ---- */

QVector<QVector<double>> ThomasAlgorithm4::solveBatch(const QVector<TridiagonalSystem>& systems)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> results;
    results.reserve(systems.size());
    for (const auto& sys : systems)
        results.append(solve(sys));

    emit batchCompleted(systems.size(), timer.elapsed());
    return results;
}

/* ---- Periodic tridiagonal (Sherman-Morrison) ---- */

QVector<double> ThomasAlgorithm4::solvePeriodic(const TridiagonalSystem& system,
                                                  double alpha, double beta)
{
    int n = system.main.size();
    if (n < 3) return {};

    // Sherman-Morrison: decompose periodic into standard + rank-1 correction
    // Modify corner elements
    QVector<double> a = system.lower;
    QVector<double> b = system.main;
    QVector<double> c = system.upper;
    QVector<double> d = system.rhs;

    double gamma = -b[0];
    b[0] -= gamma;
    b[n - 1] -= alpha * beta / gamma;

    // Build auxiliary vector u, v
    QVector<double> u(n, 0.0), v(n, 0.0);
    u[0] = gamma;
    u[n - 1] = alpha;
    v[0] = 1.0;
    v[n - 1] = beta / gamma;

    // Modify RHS
    QVector<double> dY = d;
    QVector<double> dZ(n, 0.0);
    for (int i = 0; i < n; ++i) dZ[i] = -u[i];

    // Solve two standard systems
    TridiagonalSystem sysY{a, b, c, dY};
    TridiagonalSystem sysZ{a, b, c, dZ};
    QVector<double> y = solve(sysY);
    QVector<double> z = solve(sysZ);

    // Sherman-Morrison correction: x = y + (v·y)/(1+v·z) * z
    double vy = 0.0, vz = 0.0;
    for (int i = 0; i < n; ++i) { vy += v[i] * y[i]; vz += v[i] * z[i]; }
    double factor = (qAbs(1.0 + vz) > 1e-15) ? vy / (1.0 + vz) : 0.0;

    QVector<double> x(n);
    for (int i = 0; i < n; ++i) x[i] = y[i] + factor * z[i];
    return x;
}

/* ---- Residual norm ---- */

double ThomasAlgorithm4::residualNorm(const TridiagonalSystem& system,
                                       const QVector<double>& x) const
{
    int n = system.main.size();
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = system.rhs[i] - system.main[i] * x[i];
        if (i > 0) r -= system.lower[i] * x[i - 1];
        if (i < n - 1) r -= system.upper[i] * x[i + 1];
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- Reset ---- */

void ThomasAlgorithm4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}

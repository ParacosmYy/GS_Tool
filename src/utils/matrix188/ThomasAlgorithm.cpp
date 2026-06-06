/**
 * @file ThomasAlgorithm.cpp
 * @brief ThomasAlgorithm 实现
 *
 * 实现Thomas算法：标准TDMA、周期性三对角、部分选主元。
 */

#include "utils/matrix188/ThomasAlgorithm.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm::ThomasAlgorithm(QObject *parent) : QObject(parent) {}
ThomasAlgorithm::~ThomasAlgorithm() = default;

/* ---- Configuration ---- */

void ThomasAlgorithm::setVariant(Variant v) { m_variant = v; }

/* ---- Standard TDMA ---- */

QVector<double> ThomasAlgorithm::solve(const QVector<double>& lower,
                                        const QVector<double>& main_,
                                        const QVector<double>& upper,
                                        const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    QVector<double> result(n, 0.0);
    if (n == 0) return result;

    // Copy to working arrays
    QVector<double> a = lower;
    QVector<double> b = main_;
    QVector<double> c = upper;
    QVector<double> d = rhs;

    // Pad arrays to size n (lower has n-1 elements conceptually)
    if (a.size() < n) a.resize(n, 0.0);
    if (c.size() < n) c.resize(n, 0.0);

    // Forward elimination (sweep)
    for (int i = 1; i < n; ++i) {
        if (qAbs(b[i - 1]) < 1e-15) { result.fill(0.0); return result; }
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    // Back substitution
    if (qAbs(b[n - 1]) < 1e-15) { result.fill(0.0); return result; }
    result[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        if (qAbs(b[i]) < 1e-15) { result.fill(0.0); return result; }
        result[i] = (d[i] - c[i] * result[i + 1]) / b[i];
    }

    m_stats.totalSolves++;
    m_stats.systemSize = n;
    m_stats.periodic = false;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, timer.elapsed());
    return result;
}

/* ---- Periodic TDMA (CTDMA) ---- */

QVector<double> ThomasAlgorithm::solvePeriodic(
    const QVector<double>& lower,
    const QVector<double>& main_,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    QVector<double> result(n, 0.0);
    if (n <= 2) return solve(lower, main_, upper, rhs);

    // For periodic system: a[0] = lower[n-1], c[n-1] = upper[0]
    QVector<double> a = lower;
    QVector<double> b = main_;
    QVector<double> c = upper;
    QVector<double> d = rhs;

    if (a.size() < n) a.resize(n, 0.0);
    if (c.size() < n) c.resize(n, 0.0);

    // Sherman-Morrison trick: decouple the corner elements
    // Corner: a[0] couples last row, c[n-1] couples first row
    double alpha = a[0];  // lower-left corner
    double beta = c[n - 1]; // upper-right corner

    // Modify diagonal to remove periodic coupling
    b[0] -= alpha;
    b[n - 1] -= beta * alpha / b[0];

    // Solve modified system: d' = d, and auxiliary d'' with unit spikes
    QVector<double> dMod = d;
    dMod[0] = d[0];
    dMod[n - 1] = d[n - 1] - beta * d[0] / b[0];

    // Solve the tridiagonal part
    QVector<double> x = solve(
        a.mid(1, n - 1), b, c.mid(0, n - 1), dMod);

    if (x.size() < n) x.resize(n, 0.0);

    // Solve auxiliary system with unit right-hand side
    QVector<double> e(n, 0.0);
    e[0] = alpha;
    e[n - 1] = -beta * alpha / b[0];

    QVector<double> y = solve(
        a.mid(1, n - 1), b, c.mid(0, n - 1), e);

    if (y.size() < n) y.resize(n, 0.0);

    // Combine: x_final = x - y * (x[0] + alpha*x_final[n-1]) / (1 + y[0] + alpha*y[n-1])
    // Simplified: adjust for periodicity
    double gamma = (y[0] != 0.0) ? x[0] / (1.0 + y[0]) : 0.0;
    for (int i = 0; i < n; ++i)
        result[i] = x[i] - gamma * y[i];

    m_stats.totalSolves++;
    m_stats.systemSize = n;
    m_stats.periodic = true;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, timer.elapsed());
    return result;
}

/* ---- Partial pivoting TDMA ---- */

QVector<double> ThomasAlgorithm::solveWithPivoting(
    const QVector<double>& lower,
    const QVector<double>& main_,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    QVector<double> result(n, 0.0);
    if (n == 0) return result;

    // Work with expanded tridiagonal as banded matrix (3 rows)
    QVector<double> a = lower;
    QVector<double> b = main_;
    QVector<double> c = upper;
    QVector<double> d = rhs;

    if (a.size() < n) a.resize(n, 0.0);
    if (c.size() < n) c.resize(n, 0.0);

    // Forward sweep with partial pivoting
    for (int i = 1; i < n; ++i) {
        // Check if row swap needed (compare |b[i-1]| with |a[i]|)
        if (qAbs(a[i]) > qAbs(b[i - 1])) {
            // Swap rows i-1 and i
            std::swap(a[i], b[i - 1]);
            std::swap(b[i], c[i - 1]);
            c[i] = 0.0; // After swap, the element becomes 0
            std::swap(d[i], d[i - 1]);
        }

        if (qAbs(b[i - 1]) < 1e-15) { result.fill(0.0); return result; }
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
        a[i] = 0.0;
    }

    // Back substitution
    if (qAbs(b[n - 1]) < 1e-15) { result.fill(0.0); return result; }
    result[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        if (qAbs(b[i]) < 1e-15) { result.fill(0.0); return result; }
        result[i] = (d[i] - c[i] * result[i + 1]) / b[i];
    }

    m_stats.totalSolves++;
    m_stats.systemSize = n;
    m_stats.periodic = false;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, timer.elapsed());
    return result;
}

/* ---- Main solve dispatch ---- */

// Note: the public solve() above already handles Standard variant.
// The dispatch is handled by setVariant + calling the appropriate method directly.

/* ---- Reset ---- */

void ThomasAlgorithm::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

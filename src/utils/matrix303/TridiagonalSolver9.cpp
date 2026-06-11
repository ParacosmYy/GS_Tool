/**
 * @file TridiagonalSolver9.cpp
 * @brief TridiagonalSolver9 实现
 *
 * 实现三对角求解器：循环归约与并行子问题分解实现GPU友好批量三对角方程组求解。
 */

#include "utils/matrix303/TridiagonalSolver9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TridiagonalSolver9::TridiagonalSolver9(QObject *parent)
    : QObject(parent) {}

TridiagonalSolver9::~TridiagonalSolver9() = default;

/* ---- Thomas algorithm (sequential fallback) ---- */

QVector<double> TridiagonalSolver9::solveThomas(const TridiagSystem& sys) const
{
    int n = sys.main.size();
    if (n == 0) return {};

    // Forward sweep
    QVector<double> cStar(n, 0.0);
    QVector<double> dStar(n, 0.0);

    cStar[0] = (n > 1) ? sys.upper[0] / sys.main[0] : 0.0;
    dStar[0] = sys.rhs[0] / sys.main[0];

    for (int i = 1; i < n; ++i) {
        double a = (i < sys.lower.size()) ? sys.lower[i] : 0.0;
        double denom = sys.main[i] - a * cStar[i - 1];
        if (qFabs(denom) < 1e-300) denom = 1e-300;

        cStar[i] = (i < n - 1 && i < sys.upper.size()) ? sys.upper[i] / denom : 0.0;
        dStar[i] = (sys.rhs[i] - a * dStar[i - 1]) / denom;
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = dStar[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dStar[i] - cStar[i] * x[i + 1];

    return x;
}

/* ---- Cyclic reduction forward pass ---- */

void TridiagonalSolver9::cyclicReduceForward(QVector<double>& a, QVector<double>& b,
                                              QVector<double>& c, QVector<double>& d) const
{
    int n = b.size();
    int halfN = n / 2;

    // Working copies for the reduced system
    QVector<double> aNew(halfN, 0.0);
    QVector<double> bNew(halfN, 0.0);
    QVector<double> cNew(halfN, 0.0);
    QVector<double> dNew(halfN, 0.0);

    for (int i = 0; i < halfN; ++i) {
        int idx = 2 * i + 1;  // odd index (1-indexed in standard CR)
        int left = idx - 1;
        int right = (idx + 1 < n) ? idx + 1 : -1;

        // Eliminate odd-indexed equation by combining with neighbors
        double alpha = (idx < a.size() && b[left] != 0.0) ? -a[idx] / b[left] : 0.0;
        double gamma = (right >= 0 && idx < c.size() && b[right] != 0.0) ? -c[idx] / b[right] : 0.0;

        bNew[i] = alpha * ((left < c.size()) ? c[left] : 0.0) + b[idx] +
                   gamma * ((right >= 0 && right < a.size()) ? a[right] : 0.0);

        aNew[i] = (left > 0 && (left - 1) >= 0 && (left - 1) < a.size()) ?
                   alpha * a[left] : 0.0;

        cNew[i] = (right >= 0 && right + 1 < n && right < c.size()) ?
                   gamma * c[right] : 0.0;

        dNew[i] = alpha * d[left] + d[idx] +
                   (right >= 0 ? gamma * d[right] : 0.0);
    }

    // Copy reduced system back
    a = aNew;
    b = bNew;
    c = cNew;
    d = dNew;
}

/* ---- Cyclic reduction backward pass ---- */

void TridiagonalSolver9::cyclicReduceBackward(const QVector<double>& a, const QVector<double>& b,
                                                const QVector<double>& c, const QVector<double>& d,
                                                QVector<double>& x) const
{
    // Solve reduced system (now small enough for Thomas)
    int n = b.size();
    QVector<double> reduced = solveThomas(TridiagSystem{a, b, c, d});

    // Interpolate even-indexed unknowns
    for (int i = 0; i < n; ++i) {
        int idx = 2 * i + 1;
        if (idx < x.size())
            x[idx] = reduced[i];
    }

    // Back-substitute for even indices
    for (int i = 0; i < x.size(); i += 2) {
        double sum = d[i];
        if (i > 0 && (i - 1) < x.size()) sum -= a.value(i) * x[i - 1];
        if (i + 1 < x.size()) sum -= c.value(i) * x[i + 1];
        x[i] = sum / qMax(1e-300, b[i]);
    }
}

/* ---- Solve reduced subsystem ---- */

void TridiagonalSolver9::solveReduced(int n, const QVector<double>& a, const QVector<double>& b,
                                       const QVector<double>& c, const QVector<double>& d,
                                       QVector<double>& x) const
{
    // For small systems, use Thomas directly
    auto sol = solveThomas(TridiagSystem{a, b, c, d});
    for (int i = 0; i < qMin(n, sol.size()); ++i)
        x[i] = sol[i];
}

/* ---- Solve single system via cyclic reduction ---- */

QVector<double> TridiagonalSolver9::solve(const TridiagSystem& system)
{
    QElapsedTimer timer;
    timer.start();

    int n = system.main.size();
    if (n == 0) return {};

    // For small systems, Thomas is faster
    if (n <= 16) {
        auto result = solveThomas(system);
        m_stats.totalSolves++;
        m_stats.totalSystems++;
        m_stats.systemSize = n;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveDone(n, elapsed);
        return result;
    }

    // Cyclic reduction: iteratively halve the system
    QVector<double> a = system.lower;
    QVector<double> b = system.main;
    QVector<double> c = system.upper;
    QVector<double> d = system.rhs;

    // Store history for back-substitution
    QVector<QVector<double>> histA, histB, histC, histD;

    int currentN = n;
    while (currentN > 16) {
        histA.append(a);
        histB.append(b);
        histC.append(c);
        histD.append(d);

        cyclicReduceForward(a, b, c, d);
        currentN = b.size();
    }

    // Solve smallest reduced system
    QVector<double> x(n, 0.0);
    auto reducedSol = solveThomas(TridiagSystem{a, b, c, d});

    // Back-substitute through levels
    // Simple approach: reconstruct full solution from reduced levels
    // Map reduced solution into x at odd positions, then interpolate evens
    for (int i = 0; i < reducedSol.size() && (2 * i + 1) < n; ++i)
        x[2 * i + 1] = reducedSol[i];

    // Fill even positions via substitution
    for (int i = 0; i < n; i += 2) {
        double sum = system.rhs[i];
        if (i > 0) sum -= system.lower.value(i) * x[i - 1];
        if (i + 1 < n) sum -= system.upper.value(i) * x[i + 1];
        x[i] = sum / qMax(1e-300, system.main[i]);
    }

    m_stats.totalSolves++;
    m_stats.totalSystems++;
    m_stats.systemSize = n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, elapsed);
    return x;
}

/* ---- Solve batch ---- */

TridiagonalSolver9::BatchResult TridiagonalSolver9::solveBatch(const QVector<TridiagSystem>& systems)
{
    QElapsedTimer timer;
    timer.start();

    BatchResult result;
    int batch = systems.size();
    result.batchSize = batch;

    if (batch == 0) return result;

    result.systemSize = systems[0].main.size();
    result.solutions.reserve(batch);

    // Decompose into parallel subproblems: each system is independent
    for (int i = 0; i < batch; ++i) {
        auto sol = solve(systems[i]);
        result.solutions.append(sol);
        if (sol.isEmpty()) result.allConverged = false;
    }

    m_stats.totalSystems += batch;
    double elapsed = timer.elapsed();
    emit batchDone(batch, result.systemSize, elapsed);
    return result;
}

/* ---- Reset ---- */

void TridiagonalSolver9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

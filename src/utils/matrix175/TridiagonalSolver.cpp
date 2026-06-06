/**
 * @file TridiagonalSolver.cpp
 * @brief TridiagonalSolver 实现
 *
 * 实现Thomas算法求解三对角方程组，含周期边界Sherman-Morrison变体。
 */

#include "utils/matrix175/TridiagonalSolver.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

TridiagonalSolver::TridiagonalSolver(QObject *parent)
    : QObject(parent)
{
}

TridiagonalSolver::~TridiagonalSolver() = default;

/* ---- Standard Thomas algorithm ---- */

QVector<double> TridiagonalSolver::solve(
    const QVector<double>& lower,
    const QVector<double>& main,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    if (n == 0) return QVector<double>();
    if (lower.size() != n - 1 || upper.size() != n - 1 || rhs.size() != n)
        return QVector<double>();

    /* Forward elimination */
    QVector<double> c(n - 1);  /* Modified upper diagonal */
    QVector<double> d(n);       /* Modified RHS */

    c[0] = upper[0] / main[0];
    d[0] = rhs[0] / main[0];

    for (int i = 1; i < n; ++i) {
        double a_i = (i < n - 1) ? lower[i - 1] : 0.0;
        if (i < n - 1) {
            double denom = main[i] - a_i * c[i - 1];
            c[i] = upper[i] / denom;
            d[i] = (rhs[i] - a_i * d[i - 1]) / denom;
        } else {
            double denom = main[i] - a_i * c[i - 1];
            d[i] = (rhs[i] - a_i * d[i - 1]) / denom;
        }
    }

    /* Back substitution */
    QVector<double> x(n);
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = d[i] - c[i] * x[i + 1];

    m_stats.totalSolves++;
    m_stats.lastSize = n;
    m_stats.lastPeriodic = false;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, false);
    return x;
}

/* ---- Periodic boundary (Sherman-Morrison) ---- */

QVector<double> TridiagonalSolver::solvePeriodic(
    const QVector<double>& lower,
    const QVector<double>& main,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    if (n < 3) return QVector<double>();
    if (lower.size() != n || upper.size() != n || rhs.size() != n)
        return QVector<double>();

    /*
     * Periodic tridiagonal system has corners:
     *   A[0][n-1] = lower[0]  (element at top-right)
     *   A[n-1][0] = upper[n-1] (element at bottom-left)
     *
     * Sherman-Morrison: decompose as A = A_tilde + u*v^T
     * where A_tilde is standard tridiagonal, u and v are rank-1 corrections.
     */

    double gamma = -main[0]; /* Perturbation to make A_tilde well-conditioned */

    /* Build modified tridiagonal system */
    QVector<double> modMain = main;
    QVector<double> modLower(n - 1), modUpper(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        modLower[i] = lower[i + 1]; /* Skip first lower element */
        modUpper[i] = upper[i];      /* Skip last upper element */
    }
    modMain[0] = main[0] - gamma;
    modMain[n - 1] = main[n - 1] - (upper[n - 1] * lower[0]) / gamma;

    /* Solve two standard systems: A_tilde * x = rhs, A_tilde * y = u */
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = upper[n - 1];

    QVector<double> x = solve(modLower, modMain, modUpper, rhs);
    QVector<double> y = solve(modLower, modMain, modUpper, u);

    if (x.isEmpty() || y.isEmpty()) return QVector<double>();

    /* Sherman-Morrison correction: x_final = x - (v^T * x) / (1 + v^T * y) * y */
    double vTx = x[0] + lower[0] * x[n - 1]; /* Actually: v = [1, 0,...,0, lower[0]/gamma] */
    /* Simpler: v_dot_x and v_dot_y */
    double dot_vx = x[0];
    double dot_vy = y[0];
    double factor = dot_vx / (1.0 + dot_vy);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = x[i] - factor * y[i];

    m_stats.totalSolves++;
    m_stats.lastSize = n;
    m_stats.lastPeriodic = true;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, true);
    return result;
}

/* ---- Statistics ---- */

void TridiagonalSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

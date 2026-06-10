/**
 * @file ThomasAlgorithm7.cpp
 * @brief ThomasAlgorithm7 实现
 *
 * 实现Thomas算法：Sherman-Morrison修正的周期三对角系统求解。
 */

#include "utils/matrix284/ThomasAlgorithm7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm7::ThomasAlgorithm7(QObject *parent)
    : QObject(parent) {}

ThomasAlgorithm7::~ThomasAlgorithm7() = default;

/* ---- Forward elimination ---- */

void ThomasAlgorithm7::forwardElimination(QVector<double>& a, QVector<double>& b,
                                            QVector<double>& c, QVector<double>& d) const
{
    int n = b.size();
    for (int i = 1; i < n; ++i) {
        if (qAbs(b[i - 1]) < 1e-30) return;
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
        a[i] = 0.0;
    }
}

/* ---- Back substitution ---- */

QVector<double> ThomasAlgorithm7::backSubstitution(const QVector<double>& b,
                                                     const QVector<double>& c,
                                                     const QVector<double>& d) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    if (n == 0) return x;

    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        if (qAbs(b[i]) < 1e-30) { x.fill(0.0); return x; }
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }
    return x;
}

/* ---- Solve standard tridiagonal system ---- */

ThomasAlgorithm7::SolveResult ThomasAlgorithm7::solve(const TridiagonalSystem& system)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = system.main.size();
    if (n == 0) return result;

    // Copy to working arrays (don't modify originals)
    QVector<double> a = system.lower;
    QVector<double> b = system.main;
    QVector<double> c = system.upper;
    QVector<double> d = system.rhs;

    // Pad arrays to size n (lower needs n elements, upper needs n elements)
    a.resize(n); c.resize(n); d.resize(n);

    result.size = n;
    result.success = true;

    // Forward elimination
    forwardElimination(a, b, c, d);

    // Back substitution
    result.solution = backSubstitution(b, c, d);

    // Compute residual
    result.residual = computeResidual(system, result.solution);

    double elapsed = timer.elapsed();
    m_stats.systemSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(n, result.residual, elapsed);

    return result;
}

/* ---- Solve periodic tridiagonal via Sherman-Morrison ---- */

ThomasAlgorithm7::SolveResult ThomasAlgorithm7::solvePeriodic(const PeriodicSystem& system)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = system.main.size();
    if (n < 3) return result;

    result.size = n;

    // Sherman-Morrison: decompose periodic system A = T + u*v^T
    // where T is standard tridiagonal, u and v capture corner elements
    // u = [gamma, 0, ..., 0, c_n]  v = [1, 0, ..., 0, a_1/gamma]
    double gamma = -system.main[0];  // Choose gamma = -b[0] to decouple

    // Build modified tridiagonal system T' with adjusted first and last diagonal
    TridiagonalSystem sys1;
    sys1.lower = system.lower;
    sys1.main = system.main;
    sys1.upper = system.upper;
    sys1.rhs = system.rhs;

    // Adjust diagonal: b'[0] = b[0] + gamma, b'[n-1] = b[n-1] - a_n * topLeft / gamma
    sys1.main[0] += gamma;
    // Last row adjustment: bottomRight * v[0] = bottomRight
    sys1.main[n - 1] -= system.bottomRight * system.topLeft / gamma;

    // Solve T' * x = d
    auto res1 = solve(sys1);
    if (!res1.success) { result.success = false; return result; }

    // Solve T' * y = u  (u = [gamma, 0, ..., 0, bottomRight])
    TridiagonalSystem sys2;
    sys2.lower = system.lower;
    sys2.main = system.main;
    sys2.upper = system.upper;
    sys2.rhs = QVector<double>(n, 0.0);
    sys2.rhs[0] = gamma;
    sys2.rhs[n - 1] = system.bottomRight;
    sys2.main[0] += gamma;
    sys2.main[n - 1] -= system.bottomRight * system.topLeft / gamma;

    auto res2 = solve(sys2);
    if (!res2.success) { result.success = false; return result; }

    // Sherman-Morrison correction:
    // x = x1 - (v^T * x1 / (1 + v^T * y)) * y
    // v = [topLeft/gamma, 0, ..., 0, 1]
    double vTx1 = system.topLeft * res1.solution[0] / gamma;
    if (n > 1) vTx1 += res1.solution[n - 1];

    double vTy = system.topLeft * res2.solution[0] / gamma;
    if (n > 1) vTy += res2.solution[n - 1];

    double denom = 1.0 + vTy;
    if (qAbs(denom) < 1e-30) { result.success = false; return result; }

    double factor = vTx1 / denom;
    result.solution.resize(n);
    for (int i = 0; i < n; ++i)
        result.solution[i] = res1.solution[i] - factor * res2.solution[i];

    result.success = true;

    // Compute residual for periodic system
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        if (i > 0) ax += system.lower[i] * result.solution[i - 1];
        ax += system.main[i] * result.solution[i];
        if (i < n - 1) ax += system.upper[i] * result.solution[i + 1];
        // Corner elements
        if (i == 0) ax += system.topLeft * result.solution[n - 1];
        if (i == n - 1) ax += system.bottomRight * result.solution[0];
        double r = ax - system.rhs[i];
        res += r * r;
    }
    result.residual = qSqrt(res);

    double elapsed = timer.elapsed();
    m_stats.systemSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(n, result.residual, elapsed);

    return result;
}

/* ---- Compute residual ---- */

double ThomasAlgorithm7::computeResidual(const TridiagonalSystem& system,
                                          const QVector<double>& x) const
{
    int n = system.main.size();
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = system.main[i] * x[i];
        if (i > 0) ax += system.lower[i] * x[i - 1];
        if (i < n - 1) ax += system.upper[i] * x[i + 1];
        double r = ax - system.rhs[i];
        res += r * r;
    }
    return qSqrt(res);
}

/* ---- Extract bands from dense matrix ---- */

ThomasAlgorithm7::TridiagonalSystem ThomasAlgorithm7::extractBands(
    const QVector<QVector<double>>& matrix, const QVector<double>& rhs) const
{
    int n = matrix.size();
    TridiagonalSystem sys;
    sys.main.resize(n, 0.0);
    sys.lower.resize(n, 0.0);
    sys.upper.resize(n, 0.0);
    sys.rhs = rhs;

    for (int i = 0; i < n; ++i) {
        sys.main[i] = matrix[i][i];
        if (i > 0) sys.lower[i] = matrix[i][i - 1];
        if (i < n - 1) sys.upper[i] = matrix[i][i + 1];
    }
    return sys;
}

/* ---- Reset ---- */

void ThomasAlgorithm7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

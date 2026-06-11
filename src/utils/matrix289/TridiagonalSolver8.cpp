/**
 * @file TridiagonalSolver8.cpp
 * @brief TridiagonalSolver8 实现
 *
 * 实现三对角求解器：循环约化与并行前向消元的GPU友好三对角系统求解。
 */

#include "utils/matrix289/TridiagonalSolver8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

TridiagonalSolver8::TridiagonalSolver8(QObject *parent)
    : QObject(parent) {}

TridiagonalSolver8::~TridiagonalSolver8() = default;

/* ---- Pad system to power of 2 ---- */

int TridiagonalSolver8::padToPow2(TridiagSystem& sys) const
{
    int n = sys.main.size();
    int p = 1;
    while (p < n) p <<= 1;

    if (p == n) return n;

    sys.lower.resize(p, 0.0);
    sys.main.resize(p, 1.0);
    sys.upper.resize(p, 0.0);
    sys.rhs.resize(p, 0.0);

    return p;
}

/* ---- Cyclic reduction: forward phase ---- */

void TridiagonalSolver8::crForwardReduce(QVector<double>& a, QVector<double>& b,
                                           QVector<double>& c, QVector<double>& d,
                                           int n) const
{
    int stride = 1;
    while (stride < n) {
        for (int i = stride; i < n; i += 2 * stride) {
            int left = i - stride;
            int right = qMin(i + stride, n - 1);

            double alpha = 0.0, gamma = 0.0;
            if (left >= 0 && i > 0) {
                double denom = b[left];
                if (qAbs(denom) > 1e-30) {
                    alpha = -a[i] / denom;
                }
            }
            if (i < n - 1) {
                double denom = b[right];
                if (qAbs(denom) > 1e-30) {
                    gamma = -c[i] / denom;
                }
            }

            b[i] += alpha * c[left] + gamma * a[right];
            d[i] += alpha * d[left] + gamma * d[right];

            if (i > 0) a[i] = alpha * a[left];
            if (i < n - 1) c[i] = gamma * c[right];
        }
        stride <<= 1;
    }
}

/* ---- Cyclic reduction: backward substitution ---- */

void TridiagonalSolver8::crBackSubstitute(const QVector<double>& a, const QVector<double>& b,
                                            const QVector<double>& c, const QVector<double>& d,
                                            QVector<double>& x, int n) const
{
    // After full reduction, b[n-1] contains the pivot
    x[n - 1] = d[n - 1] / qMax(qAbs(b[n - 1]), 1e-30);

    // Back-substitute in reverse stride order
    int stride = n / 2;
    while (stride >= 1) {
        for (int i = stride - 1; i < n; i += 2 * stride) {
            double sum = d[i];
            if (i > 0) sum -= a[i] * x[i - 1];
            if (i < n - 1) sum -= c[i] * x[i + 1];
            x[i] = sum / qMax(qAbs(b[i]), 1e-30);
        }
        stride >>= 1;
    }
}

/* ---- Solve via cyclic reduction ---- */

TridiagonalSolver8::SolveResult TridiagonalSolver8::solve(const TridiagSystem& sys)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = sys.main.size();
    if (n == 0) return result;

    // Copy for in-place modification
    QVector<double> a = sys.lower;
    QVector<double> b = sys.main;
    QVector<double> c = sys.upper;
    QVector<double> d = sys.rhs;

    // Pad to power of 2
    int origN = n;
    TridiagSystem padded;
    padded.lower = a; padded.main = b; padded.upper = c; padded.rhs = d;
    int paddedN = padToPow2(padded);
    a = padded.lower; b = padded.main; c = padded.upper; d = padded.rhs;
    n = paddedN;

    QVector<double> x(n, 0.0);

    crForwardReduce(a, b, c, d, n);
    crBackSubstitute(a, b, c, d, x, n);

    result.solution.resize(origN);
    for (int i = 0; i < origN; ++i)
        result.solution[i] = x[i];

    result.size = origN;
    result.residual = computeResidual(sys, result.solution);
    result.converged = (result.residual < 1e-6);

    double elapsed = timer.elapsed();
    m_stats.lastSize = origN;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(origN, result.residual, elapsed);

    return result;
}

/* ---- Thomas algorithm (sequential) ---- */

TridiagonalSolver8::SolveResult TridiagonalSolver8::solveThomas(const TridiagSystem& sys)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = sys.main.size();
    if (n == 0) return result;

    QVector<double> a = sys.lower;
    QVector<double> b = sys.main;
    QVector<double> c = sys.upper;
    QVector<double> d = sys.rhs;

    // Forward elimination
    for (int i = 1; i < n; ++i) {
        double m = a[i] / qMax(qAbs(b[i - 1]), 1e-30);
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / qMax(qAbs(b[n - 1]), 1e-30);
    for (int i = n - 2; i >= 0; --i)
        x[i] = (d[i] - c[i] * x[i + 1]) / qMax(qAbs(b[i]), 1e-30);

    result.solution = x;
    result.size = n;
    result.residual = computeResidual(sys, x);
    result.converged = (result.residual < 1e-6);

    double elapsed = timer.elapsed();
    m_stats.lastSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(n, result.residual, elapsed);

    return result;
}

/* ---- Batch solve ---- */

QVector<TridiagonalSolver8::SolveResult> TridiagonalSolver8::solveBatch(
    const QVector<TridiagSystem>& systems)
{
    QVector<SolveResult> results;
    results.reserve(systems.size());
    for (const auto& sys : systems)
        results.append(solve(sys));
    return results;
}

/* ---- Compute residual ---- */

double TridiagonalSolver8::computeResidual(const TridiagSystem& sys,
                                            const QVector<double>& x) const
{
    int n = sys.main.size();
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = sys.main[i] * x[i];
        if (i > 0) ax += sys.lower[i] * x[i - 1];
        if (i < n - 1) ax += sys.upper[i] * x[i + 1];
        double r = ax - sys.rhs[i];
        res += r * r;
    }
    return qSqrt(res);
}

/* ---- Reset ---- */

void TridiagonalSolver8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

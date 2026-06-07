/**
 * @file GaussSeidel2.cpp
 * @brief GaussSeidel2 实现
 *
 * 实现多重网格Gauss-Seidel：V循环、限制/延拓算子、粗网格校正。
 */

#include "utils/matrix205/GaussSeidel2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussSeidel2::GaussSeidel2(QObject *parent) : QObject(parent) {}
GaussSeidel2::~GaussSeidel2() = default;

/* ---- Configuration ---- */

void GaussSeidel2::setMaxIterations(int iters) { m_maxIterations = qMax(1, iters); }
void GaussSeidel2::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void GaussSeidel2::setOmega(double omega) { m_omega = qBound(0.1, omega, 2.0); }
void GaussSeidel2::setMaxLevels(int levels) { m_maxLevels = qMax(1, levels); }

/* ---- L2 norm ---- */

double GaussSeidel2::l2Norm(const QVector<double>& v)
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/* ---- Single Gauss-Seidel sweep (SOR) ---- */

void GaussSeidel2::gsSweep(QVector<double>& x, const QVector<QVector<double>>& A,
                              const QVector<double>& b) const
{
    int n = x.size();
    for (int i = 0; i < n; ++i) {
        double sigma = 0.0;
        double diag = A[i][i];
        if (qAbs(diag) < 1e-15) continue;

        for (int j = 0; j < n; ++j) {
            if (j != i) sigma += A[i][j] * x[j];
        }
        double xNew = (b[i] - sigma) / diag;
        x[i] = (1.0 - m_omega) * x[i] + m_omega * xNew;
    }
}

/* ---- Compute residual ---- */

QVector<double> GaussSeidel2::residual(const QVector<QVector<double>>& A,
                                         const QVector<double>& x,
                                         const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> r(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j) ax += A[i][j] * x[j];
        r[i] = b[i] - ax;
    }
    return r;
}

/* ---- Restrict fine to coarse (injection + averaging) ---- */

QVector<double> GaussSeidel2::restrict_(const QVector<double>& fine) const
{
    int fineN = fine.size();
    int coarseN = (fineN + 1) / 2;
    if (coarseN < 1) coarseN = 1;
    QVector<double> coarse(coarseN, 0.0);

    for (int i = 0; i < coarseN; ++i) {
        int fi = 2 * i;
        if (fi < fineN) {
            coarse[i] = fine[fi];
            // Average with neighbors
            if (fi - 1 >= 0) coarse[i] += 0.5 * fine[fi - 1];
            if (fi + 1 < fineN) coarse[i] += 0.5 * fine[fi + 1];
            coarse[i] /= 2.0;
        }
    }
    return coarse;
}

/* ---- Prolongate coarse to fine (linear interpolation) ---- */

QVector<double> GaussSeidel2::prolongate(const QVector<double>& coarse, int fineSize) const
{
    QVector<double> fine(fineSize, 0.0);
    int coarseN = coarse.size();
    if (coarseN < 1) return fine;

    for (int i = 0; i < fineSize; ++i) {
        double ci = static_cast<double>(i) * (coarseN - 1) / qMax(1, fineSize - 1);
        int lo = qBound(0, static_cast<int>(qFloor(ci)), coarseN - 1);
        int hi = qMin(lo + 1, coarseN - 1);
        double frac = ci - lo;
        fine[i] = (1.0 - frac) * coarse[lo] + frac * coarse[hi];
    }
    return fine;
}

/* ---- V-cycle ---- */

QVector<double> GaussSeidel2::vCycle(QVector<double> x, const QVector<QVector<double>>& A,
                                       const QVector<double>& b, int level) const
{
    if (level >= m_maxLevels || x.size() <= 2) {
        // Coarsest level: solve directly with Gauss-Seidel iterations
        for (int i = 0; i < 3; ++i) gsSweep(x, A, b);
        return x;
    }

    // Pre-smoothing: a few Gauss-Seidel iterations
    for (int i = 0; i < 2; ++i) gsSweep(x, A, b);

    // Compute residual
    QVector<double> r = residual(A, x, b);

    // Restrict residual to coarse grid
    QVector<double> rCoarse = restrict_(r);
    int coarseN = rCoarse.size();

    // Build coarse A (simple diagonal approximation for efficiency)
    QVector<QVector<double>> Acoarse(coarseN, QVector<double>(coarseN, 0.0));
    for (int i = 0; i < coarseN; ++i) {
        int fi = qMin(2 * i, static_cast<int>(A.size()) - 1);
        if (fi >= 0 && fi < A.size()) {
            for (int j = 0; j < coarseN; ++j) {
                int fj = qMin(2 * j, static_cast<int>(A.size()) - 1);
                Acoarse[i][j] = A[fi][fj];
            }
        }
    }

    // Solve coarse grid correction
    QVector<double> eCoarse(coarseN, 0.0);
    eCoarse = vCycle(eCoarse, Acoarse, rCoarse, level + 1);

    // Prolongate correction to fine grid
    QVector<double> eFine = prolongate(eCoarse, x.size());

    // Apply correction
    for (int i = 0; i < x.size(); ++i) x[i] += eFine[i];

    // Post-smoothing
    for (int i = 0; i < 2; ++i) gsSweep(x, A, b);

    return x;
}

/* ---- Solve ---- */

QVector<double> GaussSeidel2::solve(const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    QVector<double> x(n, 0.0);
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        x = vCycle(x, A, b, 0);

        QVector<double> r = residual(A, x, b);
        double resNorm = l2Norm(r);
        if (resNorm < m_tolerance) break;
    }

    m_stats.totalSolves++;
    m_stats.gridSize = n;
    m_stats.iterations = iter;
    m_stats.residual = l2Norm(residual(A, x, b));
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, m_stats.residual, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void GaussSeidel2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file ConjugateGradient3.cpp
 * @brief ConjugateGradient3 实现
 *
 * 实现AMG预条件共轭梯度法：层次网格构造、粗化/插值、V-cycle。
 */

#include "utils/matrix207/ConjugateGradient3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ConjugateGradient3::ConjugateGradient3(QObject *parent) : QObject(parent) {}
ConjugateGradient3::~ConjugateGradient3() = default;

/* ---- Configuration ---- */

void ConjugateGradient3::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void ConjugateGradient3::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void ConjugateGradient3::setAmgLevels(int levels) { m_amgLevels = qMax(1, levels); }

/* ---- Dot product ---- */

double ConjugateGradient3::dot(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

/* ---- Matrix-vector product ---- */

QVector<double> ConjugateGradient3::matVec(const QVector<QVector<double>>& A, const QVector<double>& x)
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(A[i].size(), n); ++j)
            result[i] += A[i][j] * x[j];
    return result;
}

/* ---- Gauss-Seidel smoother ---- */

void ConjugateGradient3::gaussSeidel(const QVector<QVector<double>>& A,
                                        QVector<double>& x, const QVector<double>& b, int sweeps)
{
    int n = A.size();
    for (int s = 0; s < sweeps; ++s) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            double diag = 1.0;
            for (int j = 0; j < qMin(A[i].size(), n); ++j) {
                if (j == i) diag = qMax(qAbs(A[i][j]), 1e-15);
                else sigma += A[i][j] * x[j];
            }
            x[i] = (b[i] - sigma) / diag;
        }
    }
}

/* ---- Transpose ---- */

QVector<QVector<double>> ConjugateGradient3::transpose(const QVector<QVector<double>>& P)
{
    if (P.isEmpty()) return {};
    int rows = P.size();
    int cols = P[0].size();
    QVector<QVector<double>> T(cols, QVector<double>(rows, 0.0));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            T[j][i] = P[i][j];
    return T;
}

/* ---- Coarsening ---- */

QVector<int> ConjugateGradient3::coarsen(const QVector<QVector<double>>& mat) const
{
    int n = mat.size();
    QVector<bool> isCoarse(n, false);
    QVector<bool> isFine(n, false);

    // Standard coarsening: strong connection threshold
    double threshold = 0.25;
    for (int i = 0; i < n; ++i) {
        if (isFine[i]) continue;

        // Find max off-diagonal in row i
        double maxOff = 0.0;
        for (int j = 0; j < qMin(mat[i].size(), n); ++j) {
            if (j != i) maxOff = qMax(maxOff, qAbs(mat[i][j]));
        }

        // Mark strong connections
        for (int j = i + 1; j < qMin(mat[i].size(), n); ++j) {
            if (qAbs(mat[i][j]) >= threshold * maxOff && !isFine[j]) {
                isCoarse[j] = true;
                // Mark neighbors as fine
                isFine[i] = true;
                break;
            }
        }
    }

    // Ensure at least some coarse points
    if (!isCoarse.contains(true) && n > 0) isCoarse[0] = true;

    QVector<int> coarseSet;
    for (int i = 0; i < n; ++i)
        if (isCoarse[i]) coarseSet.append(i);
    return coarseSet;
}

/* ---- Build interpolation ---- */

QVector<QVector<double>> ConjugateGradient3::buildInterpolation(const QVector<QVector<double>>& mat,
                                                                   const QVector<int>& coarseSet) const
{
    int n = mat.size();
    int nc = coarseSet.size();
    QVector<QVector<double>> P(n, QVector<double>(nc, 0.0));

    QVector<int> coarseIdx(n, -1);
    for (int i = 0; i < nc; ++i) coarseIdx[coarseSet[i]] = i;

    // Direct injection for coarse points
    for (int c = 0; c < nc; ++c)
        P[coarseSet[c]][c] = 1.0;

    // Linear interpolation for fine points
    for (int i = 0; i < n; ++i) {
        if (coarseIdx[i] >= 0) continue; // already a coarse point

        // Find nearest coarse neighbors
        double sumWeights = 0.0;
        QVector<double> weights(nc, 0.0);
        for (int c = 0; c < nc; ++c) {
            int j = coarseSet[c];
            if (j < mat[i].size()) {
                double w = qAbs(mat[i][j]);
                weights[c] = w;
                sumWeights += w;
            }
        }
        if (sumWeights > 1e-15) {
            for (int c = 0; c < nc; ++c)
                P[i][c] = weights[c] / sumWeights;
        } else if (nc > 0) {
            P[i][0] = 1.0; // fallback
        }
    }
    return P;
}

/* ---- Build AMG hierarchy ---- */

void ConjugateGradient3::buildAMG(const QVector<QVector<double>>& A)
{
    m_Alevels.clear();
    m_Plevels.clear();
    m_Rlevels.clear();

    m_Alevels.append(A);

    for (int level = 0; level < m_amgLevels - 1; ++level) {
        const auto& curA = m_Alevels[level];
        if (curA.size() < 4) break; // too small to coarsen

        QVector<int> coarseSet = coarsen(curA);
        if (coarseSet.size() >= curA.size()) break; // no reduction

        QVector<QVector<double>> P = buildInterpolation(curA, coarseSet);
        QVector<QVector<double>> R = transpose(P);

        // Galerkin product: A_coarse = R * A_fine * P
        int nc = coarseSet.size();
        QVector<QVector<double>> Acoarse(nc, QVector<double>(nc, 0.0));
        for (int i = 0; i < nc; ++i)
            for (int j = 0; j < nc; ++j)
                for (int k = 0; k < curA.size(); ++k)
                    for (int l = 0; l < qMin(curA[k].size(), P[l % P.size()].size()); ++l)
                        Acoarse[i][j] += R[i][k] * curA[k][l] * P[l][j];

        m_Plevels.append(P);
        m_Rlevels.append(R);
        m_Alevels.append(Acoarse);
    }
}

/* ---- V-cycle ---- */

QVector<double> ConjugateGradient3::amgVCycle(const QVector<double>& residual, int level) const
{
    if (level >= m_Alevels.size() || m_Alevels[level].isEmpty())
        return QVector<double>(residual.size(), 0.0);

    const auto& A = m_Alevels[level];
    int n = A.size();

    // Pre-smoothing
    QVector<double> x(n, 0.0);
    QVector<double> b = residual;
    if (b.size() > n) b.resize(n);
    gaussSeidel(A, x, b, 2);

    // Compute residual
    QVector<double> Ax = matVec(A, x);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i)
        r[i] = (i < b.size() ? b[i] : 0.0) - (i < Ax.size() ? Ax[i] : 0.0);

    // Coarsen residual
    if (level < m_Plevels.size()) {
        const auto& R = m_Rlevels[level];
        int nc = R.size();
        QVector<double> rc(nc, 0.0);
        for (int i = 0; i < nc; ++i)
            for (int j = 0; j < qMin(R[i].size(), r.size()); ++j)
                rc[i] += R[i][j] * r[j];

        // Recursive coarse grid correction
        QVector<double> ec = amgVCycle(rc, level + 1);

        // Interpolate correction
        const auto& P = m_Plevels[level];
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < qMin(P[i].size(), ec.size()); ++j)
                x[i] += P[i][j] * ec[j];
    }

    // Post-smoothing
    gaussSeidel(A, x, b, 2);
    return x;
}

/* ---- Solve ---- */

QVector<double> ConjugateGradient3::solve(const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};

    // Build AMG hierarchy
    buildAMG(A);

    // Initial guess x = 0
    QVector<double> x(n, 0.0);
    QVector<double> r = b; // r = b - A*0 = b

    // Apply preconditioner: z = M^{-1} r
    QVector<double> z = amgVCycle(r, 0);
    if (z.size() < n) z.resize(n);

    QVector<double> p = z;
    double rzOld = dot(r, z);

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> Ap = matVec(A, p);
        if (Ap.size() < n) Ap.resize(n);

        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-30) break;

        double alpha = rzOld / pAp;
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rNorm = qSqrt(dot(r, r));
        if (rNorm < m_tol) break;

        z = amgVCycle(r, 0);
        if (z.size() < n) z.resize(n);

        double rzNew = dot(r, z);
        double beta = rzNew / qMax(rzOld, 1e-30);
        for (int i = 0; i < n; ++i)
            p[i] = z[i] + beta * p[i];
        rzOld = rzNew;
    }

    double finalRes = qSqrt(dot(r, r));
    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.iterationsUsed = iter;
    m_stats.residualNorm = finalRes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, iter, finalRes, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void ConjugateGradient3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_Alevels.clear();
    m_Plevels.clear();
    m_Rlevels.clear();
}

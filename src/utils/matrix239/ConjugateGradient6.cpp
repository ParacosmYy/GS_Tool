/**
 * @file ConjugateGradient6.cpp
 * @brief ConjugateGradient6 实现
 *
 * 实现共轭梯度法：不完全Cholesky预处理与柔性CG变预处理求解。
 */

#include "utils/matrix239/ConjugateGradient6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ConjugateGradient6::ConjugateGradient6(QObject *parent) : QObject(parent) {}
ConjugateGradient6::~ConjugateGradient6() = default;

/* ---- Configuration ---- */

void ConjugateGradient6::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void ConjugateGradient6::setTolerance(double tol) { m_tolerance = qMax(1e-14, tol); }
void ConjugateGradient6::setFlexibleMode(bool flexible) { m_flexibleMode = flexible; }

/* ---- Vector ops ---- */

double ConjugateGradient6::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double ConjugateGradient6::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Dense matrix-vector multiply ---- */

QVector<double> ConjugateGradient6::denseMV(const QVector<QVector<double>>& A,
                                            const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(A[i].size(), n); ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Sparse matrix-vector multiply ---- */

QVector<double> ConjugateGradient6::sparseMV(const QVector<QVector<SparseEntry>>& A,
                                             const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (const auto& e : A[i])
            y[i] += e.val * x[e.col];
    return y;
}

/* ---- Incomplete Cholesky factorization ---- */

QVector<QVector<ConjugateGradient6::SparseEntry>>
ConjugateGradient6::incompleteCholesky(const QVector<QVector<SparseEntry>>& A) const
{
    int n = A.size();
    // Build dense lower triangle for factorization
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    // Copy lower triangle from sparse A
    for (int i = 0; i < n; ++i) {
        for (const auto& e : A[i]) {
            if (e.col <= i) L[i][e.col] = e.val;
        }
    }

    // IC(0): incomplete Cholesky, drop fill-in outside sparsity pattern
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < i; ++k) {
            if (qAbs(L[i][k]) < 1e-15) continue;
            L[i][k] /= L[k][k];
            for (int j = k + 1; j <= i; ++j) {
                // Only update if (i,j) exists in original sparsity
                if (qAbs(L[i][j]) > 1e-15 || j == i)
                    L[i][j] -= L[i][k] * L[j][k];
            }
        }
        // Ensure positive diagonal
        if (L[i][i] <= 0.0) L[i][i] = 1e-6;
        L[i][i] = qSqrt(L[i][i]);
    }

    // Convert back to sparse
    QVector<QVector<SparseEntry>> Ls(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (qAbs(L[i][j]) > 1e-15)
                Ls[i].append({j, L[i][j]});
        }
    }
    return Ls;
}

/* ---- Forward solve L*y = b ---- */

QVector<double> ConjugateGradient6::forwardSolve(const QVector<QVector<SparseEntry>>& L,
                                                  const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (const auto& e : L[i]) {
            if (e.col < i) sum -= e.val * y[e.col];
        }
        // Diagonal is last entry with col == i
        double diag = 1.0;
        for (const auto& e : L[i]) {
            if (e.col == i) { diag = e.val; break; }
        }
        y[i] = sum / diag;
    }
    return y;
}

/* ---- Backward solve L^T*x = y ---- */

QVector<double> ConjugateGradient6::backwardSolve(const QVector<QVector<SparseEntry>>& L,
                                                   const QVector<double>& y) const
{
    int n = y.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        // L^T: column i of L^T = row i of L
        // Sum contributions from rows j > i where L[j] has entry at col i
        for (int j = i + 1; j < n; ++j) {
            for (const auto& e : L[j]) {
                if (e.col == i) sum -= e.val * x[j];
            }
        }
        double diag = 1.0;
        for (const auto& e : L[i]) {
            if (e.col == i) { diag = e.val; break; }
        }
        x[i] = sum / diag;
    }
    return x;
}

/* ---- Solve dense ---- */

ConjugateGradient6::SolveResult ConjugateGradient6::solveDense(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    SolveResult result;
    result.x.resize(n, 0.0);
    if (n == 0) return result;

    // r = b - A*x (x = 0 initially)
    QVector<double> r = b;
    QVector<double> p = r;
    double rsOld = dot(r, r);
    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        QVector<double> Ap = denseMV(A, p);
        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-15) break;

        double alpha = rsOld / pAp;

        // x = x + alpha*p
        for (int i = 0; i < n; ++i) {
            result.x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rsNew = dot(r, r);
        result.iterations = iter + 1;
        result.finalResidual = qSqrt(rsNew) / bNorm;

        emit iterationAdvanced(iter + 1, result.finalResidual);

        if (result.finalResidual < m_tolerance) {
            result.converged = true;
            break;
        }

        double beta = rsNew / rsOld;
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * p[i];
        rsOld = rsNew;
    }

    m_stats.matrixSize = n;
    m_stats.totalIterations += result.iterations;
    if (result.converged) m_stats.numConverged++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.finalResidual, result.converged);
    return result;
}

/* ---- Solve sparse with preconditioner ---- */

ConjugateGradient6::SolveResult ConjugateGradient6::solveSparse(
    const QVector<QVector<SparseEntry>>& A, const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    SolveResult result;
    result.x.resize(n, 0.0);
    if (n == 0) return result;

    // Compute IC preconditioner
    QVector<QVector<SparseEntry>> L = incompleteCholesky(A);

    // r = b - A*x
    QVector<double> r = b;

    // Precondition: z = M^{-1} * r
    QVector<double> y = forwardSolve(L, r);
    QVector<double> z = backwardSolve(L, y);
    QVector<double> p = z;

    double rzOld = dot(r, z);
    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;

    // Previous p for flexible CG
    QVector<double> pPrev;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        QVector<double> Ap = sparseMV(A, p);
        double pAp = dot(p, Ap);
        if (qAbs(pAp) < 1e-15) break;

        double alpha = rzOld / pAp;

        for (int i = 0; i < n; ++i) {
            result.x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        // Apply preconditioner again
        QVector<double> yNew = forwardSolve(L, r);
        QVector<double> zNew = backwardSolve(L, yNew);

        double rzNew = dot(r, zNew);
        result.iterations = iter + 1;
        result.finalResidual = norm(r) / bNorm;

        emit iterationAdvanced(iter + 1, result.finalResidual);

        if (result.finalResidual < m_tolerance) {
            result.converged = true;
            break;
        }

        double beta;
        if (m_flexibleMode) {
            // Flexible CG: use different beta formula
            // For variable preconditioning stability
            QVector<double> diff(n);
            for (int i = 0; i < n; ++i) diff[i] = zNew[i] - z[i];
            beta = (dot(r, zNew) - dot(r, diff)) / rzOld;
            beta = qMax(0.0, beta);  // Clamp for stability
        } else {
            beta = rzNew / rzOld;
        }

        for (int i = 0; i < n; ++i)
            p[i] = zNew[i] + beta * p[i];
        z = zNew;
        rzOld = rzNew;
    }

    m_stats.matrixSize = n;
    m_stats.totalIterations += result.iterations;
    if (result.converged) m_stats.numConverged++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(result.iterations, result.finalResidual, result.converged);
    return result;
}

/* ---- Reset ---- */

void ConjugateGradient6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

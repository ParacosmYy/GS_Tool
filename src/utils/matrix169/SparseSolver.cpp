/**
 * @file SparseSolver.cpp
 * @brief SparseSolver 实现
 *
 * 实现PCG求解器：CSR稀疏矩阵存储、不完全Cholesky分解IC(0)预处理、
 * 共轭梯度迭代求解。
 */

#include "utils/matrix169/SparseSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

SparseSolver::SparseSolver(QObject* parent)
    : QObject(parent)
{
}

SparseSolver::~SparseSolver() = default;

void SparseSolver::setMaxIterations(int maxIter) { m_maxIterations = qMax(1, maxIter); }
void SparseSolver::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }

void SparseSolver::spmv(const SparseMatrix& A, const QVector<double>& x, QVector<double>& y)
{
    const int n = A.n;
    y.resize(n);
    y.fill(0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = A.rowPtr[i]; k < A.rowPtr[i + 1]; ++k) {
            sum += A.values[k] * x[A.colIndices[k]];
        }
        y[i] = sum;
    }
}

void SparseSolver::incompleteCholesky(const SparseMatrix& A, SparseMatrix& L) const
{
    const int n = A.n;
    L.n = n;
    L.rowPtr.resize(n + 1, 0);
    L.values.clear();
    L.colIndices.clear();

    /* Build L with same sparsity pattern as lower triangle of A */
    QVector<QVector<int>> cols(n);
    QVector<QVector<double>> vals(n);

    for (int i = 0; i < n; ++i) {
        cols[i].append(i);
        vals[i].append(A.values[A.rowPtr[i]]); /* diagonal */

        for (int k = A.rowPtr[i] + 1; k < A.rowPtr[i + 1]; ++k) {
            int j = A.colIndices[k];
            if (j < i) {
                cols[i].append(j);
                vals[i].append(A.values[k]);
            }
        }
    }

    /* IC(0) factorization in place */
    QVector<double> diag(n, 0.0);
    for (int i = 0; i < n; ++i) {
        /* Diagonal */
        double sum = 0.0;
        for (int k = 1; k < cols[i].size(); ++k) {
            int j = cols[i][k];
            sum += vals[i][k] * vals[i][k];
        }
        diag[i] = qSqrt(qMax(1e-14, vals[i][0] - sum));
        vals[i][0] = diag[i];

        /* Update columns below */
        for (int k = 1; k < cols[i].size(); ++k) {
            int j = cols[i][k];
            /* Find a_ij in row j */
            double aij = vals[i][k];
            vals[i][k] /= diag[i];

            /* Update L[j][j] and below */
            for (int m = 1; m < cols[j].size(); ++m) {
                if (cols[j][m] == i) {
                    /* Find corresponding entry in row i */
                    double lij = 0.0;
                    for (int p = 1; p < cols[i].size(); ++p) {
                        if (cols[i][p] == cols[j][m]) {
                            lij = vals[i][p];
                            break;
                        }
                    }
                    vals[j][m] -= aij * lij;
                    break;
                }
            }
        }
    }

    /* Build CSR */
    L.rowPtr[0] = 0;
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < cols[i].size(); ++k) {
            L.colIndices.append(cols[i][k]);
            L.values.append(vals[i][k]);
        }
        L.rowPtr[i + 1] = L.colIndices.size();
    }
}

void SparseSolver::forwardSolve(const SparseMatrix& L, const QVector<double>& r,
                                QVector<double>& y) const
{
    const int n = L.n;
    y.resize(n);
    y.fill(0.0);
    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (int k = L.rowPtr[i]; k < L.rowPtr[i + 1]; ++k) {
            int j = L.colIndices[k];
            if (j < i) sum -= L.values[k] * y[j];
        }
        /* Diagonal is first entry */
        double diag = L.values[L.rowPtr[i]];
        y[i] = (diag > 1e-15) ? sum / diag : sum;
    }
}

void SparseSolver::backwardSolve(const SparseMatrix& L, const QVector<double>& y,
                                 QVector<double>& x) const
{
    const int n = L.n;
    x.resize(n);
    x.fill(0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j) {
            /* Find L[j][i] */
            for (int k = L.rowPtr[j]; k < L.rowPtr[j + 1]; ++k) {
                if (L.colIndices[k] == i) { sum -= L.values[k] * x[j]; break; }
            }
        }
        double diag = L.values[L.rowPtr[i]];
        x[i] = (diag > 1e-15) ? sum / diag : sum;
    }
}

SparseSolver::SolveResult SparseSolver::solve(const SparseMatrix& A, const QVector<double>& b,
                                               const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    const int n = A.n;
    SolveResult result;
    result.converged = false;
    result.iterations = 0;

    if (n == 0 || b.size() < n) {
        result.x = QVector<double>(n, 0.0);
        return result;
    }

    /* Initial guess */
    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;

    /* Compute initial residual: r = b - A*x */
    QVector<double> Ax;
    spmv(A, x, Ax);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];

    /* Precondition: compute z = M^{-1} * r */
    SparseMatrix L;
    incompleteCholesky(A, L);
    QVector<double> y, z;
    forwardSolve(L, r, y);
    backwardSolve(L, y, z);

    QVector<double> p = z;
    double rz = 0.0;
    for (int i = 0; i < n; ++i) rz += r[i] * z[i];

    double bNorm = 0.0;
    for (int i = 0; i < n; ++i) bNorm += b[i] * b[i];
    bNorm = qSqrt(bNorm);
    if (bNorm < 1e-15) bNorm = 1.0;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        result.iterations = iter + 1;

        /* q = A * p */
        QVector<double> q;
        spmv(A, p, q);

        /* alpha = rz / p^T * q */
        double pq = 0.0;
        for (int i = 0; i < n; ++i) pq += p[i] * q[i];
        if (qAbs(pq) < 1e-30) break;
        double alpha = rz / pq;

        /* Update x and r */
        double rNorm = 0.0;
        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * q[i];
            rNorm += r[i] * r[i];
        }
        rNorm = qSqrt(rNorm);

        if (rNorm / bNorm < m_tolerance) {
            result.converged = true;
            break;
        }

        /* Precondition: z = M^{-1} * r */
        forwardSolve(L, r, y);
        backwardSolve(L, y, z);

        double rzNew = 0.0;
        for (int i = 0; i < n; ++i) rzNew += r[i] * z[i];

        double beta = rzNew / rz;
        for (int i = 0; i < n; ++i) p[i] = z[i] + beta * p[i];
        rz = rzNew;
    }

    /* Compute final residual */
    QVector<double> finalAx;
    spmv(A, x, finalAx);
    double res = 0.0;
    for (int i = 0; i < n; ++i) {
        double d = b[i] - finalAx[i];
        res += d * d;
    }
    result.residual = qSqrt(res);
    result.x = x;

    m_stats.totalSolves++;
    m_stats.lastIterations = result.iterations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(result.iterations, result.residual);
    return result;
}

void SparseSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

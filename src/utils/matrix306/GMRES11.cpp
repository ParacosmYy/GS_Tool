/**
 * @file GMRES11.cpp
 * @brief GMRES11 实现
 *
 * 实现GMRES求解器：灵活预处理与增强Krylov子空间实现跨多次求解的基向量高效复用。
 */

#include "utils/matrix306/GMRES11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GMRES11::GMRES11(QObject *parent)
    : QObject(parent) {}

GMRES11::~GMRES11() = default;

/* ---- Configuration ---- */

void GMRES11::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 100000); }
void GMRES11::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }
void GMRES11::setRestartInterval(int m) { m_restart = qBound(5, m, 200); }
void GMRES11::setAugmentedVectors(const QVector<QVector<double>>& vecs) { m_augVecs = vecs; }

/* ---- Sparse matrix-vector product ---- */

QVector<double> GMRES11::spmv(const SparseMatrix& A, const QVector<double>& x) const
{
    int n = A.rows;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = A.rowPtr[i]; j < A.rowPtr[i + 1]; ++j)
            sum += A.values[j] * x[A.colIndices[j]];
        y[i] = sum;
    }
    return y;
}

/* ---- Jacobi-like flexible preconditioner ---- */

QVector<double> GMRES11::precondition(const SparseMatrix& A, const QVector<double>& r) const
{
    int n = r.size();
    QVector<double> z(n, 0.0);
    // Jacobi preconditioner: z_i = r_i / A_ii
    for (int i = 0; i < n; ++i) {
        double diag = 0.0;
        for (int j = A.rowPtr[i]; j < A.rowPtr[i + 1]; ++j) {
            if (A.colIndices[j] == i) { diag = A.values[j]; break; }
        }
        z[i] = (qAbs(diag) > 1e-15) ? r[i] / diag : r[i];
    }
    return z;
}

/* ---- Vector operations ---- */

double GMRES11::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double GMRES11::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Solve upper Hessenberg via Givens rotations ---- */

QVector<double> GMRES11::solveHessenberg(const QVector<QVector<double>>& H,
                                          const QVector<double>& g, int m) const
{
    // Back-substitution on upper triangular
    QVector<double> y(m, 0.0);
    for (int i = m - 1; i >= 0; --i) {
        y[i] = g[i];
        for (int j = i + 1; j < m; ++j)
            y[i] -= H[j][i] * y[j];
        if (qAbs(H[i][i]) > 1e-15)
            y[i] /= H[i][i];
    }
    return y;
}

/* ---- Main solve ---- */

GMRES11::SolveResult GMRES11::solve(const SparseMatrix& A, const QVector<double>& b)
{
    QVector<double> x0(b.size(), 0.0);
    return solveWithGuess(A, b, x0);
}

/* ---- Solve with initial guess ---- */

GMRES11::SolveResult GMRES11::solveWithGuess(const SparseMatrix& A,
                                              const QVector<double>& b,
                                              const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0 || A.rows != n) return result;

    QVector<double> x = x0;

    // Compute initial residual r = b - A*x
    QVector<double> Ax = spmv(A, x);
    QVector<double> r(n);
    for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];

    double beta = norm(r);
    double bNorm = norm(b);
    if (bNorm < 1e-15) bNorm = 1.0;

    // Add augmented vectors to initial Krylov subspace
    int augSize = m_augVecs.size();
    int totalRestart = m_restart + augSize;

    int totalIter = 0;
    bool converged = false;

    for (int outer = 0; outer < m_maxIter / qMax(1, m_restart); ++outer) {
        if (beta / bNorm < m_tol) { converged = true; break; }

        // Allocate Arnoldi matrices
        QVector<QVector<double>> V(totalRestart + 1, QVector<double>(n, 0.0));
        QVector<QVector<double>> H(totalRestart + 1, QVector<double>(totalRestart, 0.0));
        QVector<double> g(totalRestart + 1, 0.0);

        // Givens rotation cos/sin
        QVector<double> cs(totalRestart, 0.0);
        QVector<double> sn(totalRestart, 0.0);

        // V[0] = r / beta
        for (int i = 0; i < n; ++i) V[0][i] = r[i] / qMax(1e-15, beta);
        g[0] = beta;

        // Add augmented vectors to Krylov basis
        int startJ = 0;
        for (int a = 0; a < augSize && a < totalRestart; ++a) {
            V[a + 1] = m_augVecs[a];
            // Orthogonalize against previous
            for (int k = 0; k <= a; ++k) {
                double h = dot(V[k], V[a + 1]);
                H[a][k] = h;
                for (int i = 0; i < n; ++i) V[a + 1][i] -= h * V[k][i];
            }
            double nv = norm(V[a + 1]);
            if (nv < 1e-15) { startJ = a; break; }
            for (int i = 0; i < n; ++i) V[a + 1][i] /= nv;
            startJ = a + 1;
        }

        // Arnoldi iteration with flexible preconditioning
        int j = startJ;
        for (; j < totalRestart && totalIter < m_maxIter; ++j, ++totalIter) {
            // w = A * V[j]  (with flexible preconditioning)
            QVector<double> Av = spmv(A, V[j]);
            QVector<double> w = precondition(A, Av);

            // Modified Gram-Schmidt
            for (int k = 0; k <= j; ++k) {
                double h = dot(V[k], w);
                H[j][k] = h;
                for (int i = 0; i < n; ++i) w[i] -= h * V[k][i];
            }
            double hw = norm(w);
            H[j][j + 1] = hw;

            if (hw < 1e-15) break;

            for (int i = 0; i < n; ++i) V[j + 1][i] = w[i] / hw;

            // Apply previous Givens rotations
            for (int k = 0; k < j; ++k) {
                double temp = cs[k] * H[j][k] + sn[k] * H[j][k + 1];
                H[j][k + 1] = -sn[k] * H[j][k] + cs[k] * H[j][k + 1];
                H[j][k] = temp;
            }

            // New Givens rotation
            double rr = qSqrt(H[j][j] * H[j][j] + H[j][j + 1] * H[j][j + 1]);
            if (rr < 1e-15) break;
            cs[j] = H[j][j] / rr;
            sn[j] = H[j][j + 1] / rr;
            H[j][j] = rr;
            H[j][j + 1] = 0.0;

            g[j + 1] = -sn[j] * g[j];
            g[j] = cs[j] * g[j];

            // Check convergence
            if (qAbs(g[j + 1]) / bNorm < m_tol) {
                converged = true;
                j++;
                break;
            }
        }

        // Solve least squares: y = H\g
        QVector<double> y = solveHessenberg(H, g, j);

        // Update x: x = x + V(:,1:j) * y
        for (int k = 0; k < j; ++k)
            for (int i = 0; i < n; ++i)
                x[i] += V[k][i] * y[k];

        // Compute new residual
        QVector<double> Axn = spmv(A, x);
        for (int i = 0; i < n; ++i) r[i] = b[i] - Axn[i];
        beta = norm(r);

        // Update augmented vectors from latest Krylov vectors
        m_augVecs.clear();
        for (int k = qMax(0, j - 3); k < j; ++k)
            m_augVecs.append(V[k]);

        if (converged) break;
    }

    result.solution = x;
    result.residual = beta / bNorm;
    result.iterations = totalIter;
    result.converged = converged;
    result.elapsedMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.maxIterations = qMax(m_stats.maxIterations, totalIter);
    m_resSum += result.residual;
    m_stats.avgResidual = m_resSum / m_stats.totalSolves;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(totalIter, result.residual, result.elapsedMs);
    return result;
}

/* ---- Reset ---- */

void GMRES11::resetStatistics()
{
    m_stats = Stats{};
    m_resSum = 0.0;
    m_timeSum = 0.0;
    m_augVecs.clear();
}

/**
 * @file ConjugateGradient10.cpp
 * @brief ConjugateGradient10 实现
 *
 * 实现共轭梯度法：预处理多项式与自适应步长控制实现对称正定系统求解。
 */

#include "utils/matrix295/ConjugateGradient10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ConjugateGradient10::ConjugateGradient10(QObject *parent)
    : QObject(parent) {}

ConjugateGradient10::~ConjugateGradient10() = default;

/* ---- Configuration ---- */

void ConjugateGradient10::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 1000000); }
void ConjugateGradient10::setTolerance(double tol) { m_tol = qBound(1e-15, tol, 1.0); }
void ConjugateGradient10::setPreconditioner(Preconditioner prec) { m_prec = prec; }
void ConjugateGradient10::setSSORParameter(double omega) { m_omega = qBound(0.5, omega, 2.0); }

/* ---- Matrix-vector multiply ---- */

QVector<double> ConjugateGradient10::matVecMul(const QVector<QVector<double>>& A,
                                                 const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Dot product ---- */

double ConjugateGradient10::dotProduct(const QVector<double>& a,
                                         const QVector<double>& b) const
{
    double d = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        d += a[i] * b[i];
    return d;
}

/* ---- Compute residual ---- */

QVector<double> ConjugateGradient10::computeResidual(const QVector<QVector<double>>& A,
                                                       const QVector<double>& x,
                                                       const QVector<double>& b) const
{
    QVector<double> r = b;
    QVector<double> Ax = matVecMul(A, x);
    for (int i = 0; i < r.size(); ++i)
        r[i] -= Ax[i];
    return r;
}

/* ---- Build Jacobi preconditioner ---- */

QVector<double> ConjugateGradient10::buildJacobiPrecond(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<double> diag(n);
    for (int i = 0; i < n; ++i)
        diag[i] = (qAbs(A[i][i]) > 1e-15) ? 1.0 / A[i][i] : 1.0;
    return diag;
}

/* ---- Build SSOR preconditioner ---- */

QVector<QVector<double>> ConjugateGradient10::buildSSORPrecond(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    // Return D/omega where D is diagonal of A (simplified SSOR)
    QVector<QVector<double>> M(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        M[i][i] = A[i][i] / m_omega;
    return M;
}

/* ---- Build IC0 (incomplete Cholesky) preconditioner ---- */

QVector<QVector<double>> ConjugateGradient10::buildIC0Precond(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<QVector<double>> L(n, QVector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (j == i) {
                double sum = A[i][i];
                for (int k = 0; k < i; ++k)
                    sum -= L[i][k] * L[i][k];
                L[i][i] = qSqrt(qMax(sum, 1e-15));
            } else if (A[i][j] != 0.0) {
                // Only fill where A has nonzero entries (IC0 pattern)
                double sum = A[i][j];
                for (int k = 0; k < j; ++k)
                    sum -= L[i][k] * L[j][k];
                if (qAbs(L[j][j]) > 1e-15)
                    L[i][j] = sum / L[j][j];
            }
        }
    }
    return L;
}

/* ---- Forward solve Lx = b ---- */

QVector<double> ConjugateGradient10::forwardSolve(const QVector<QVector<double>>& L,
                                                    const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= L[i][j] * x[j];
        x[i] = (qAbs(L[i][i]) > 1e-15) ? sum / L[i][i] : 0.0;
    }
    return x;
}

/* ---- Back solve Ux = b ---- */

QVector<double> ConjugateGradient10::backSolve(const QVector<QVector<double>>& U,
                                                 const QVector<double>& b) const
{
    int n = b.size();
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        for (int j = i + 1; j < n; ++j)
            sum -= U[i][j] * x[j];
        x[i] = (qAbs(U[i][i]) > 1e-15) ? sum / U[i][i] : 0.0;
    }
    return x;
}

/* ---- Apply preconditioner ---- */

QVector<double> ConjugateGradient10::applyPreconditioner(const QVector<double>& r,
                                                           const QVector<QVector<double>>& A) const
{
    if (m_prec == None) return r;

    if (m_prec == Jacobi) {
        auto diag = buildJacobiPrecond(A);
        QVector<double> z(r.size());
        for (int i = 0; i < r.size(); ++i)
            z[i] = diag[i] * r[i];
        return z;
    }

    if (m_prec == SSOR) {
        auto M = buildSSORPrecond(A);
        return forwardSolve(M, r);
    }

    if (m_prec == IC0) {
        auto L = buildIC0Precond(A);
        // Build L^T
        int n = L.size();
        QVector<QVector<double>> Lt(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                Lt[i][j] = L[j][i];
        auto y = forwardSolve(L, r);
        return backSolve(Lt, y);
    }

    return r;
}

/* ---- Adaptive step-length ---- */

double ConjugateGradient10::adaptiveStepLength(const QVector<double>& p,
                                                 const QVector<double>& Ap,
                                                 double rr) const
{
    double pAp = dotProduct(p, Ap);
    if (qAbs(pAp) < 1e-30) return 0.0;
    return rr / pAp;
}

/* ---- Main solve ---- */

ConjugateGradient10::SolveResult ConjugateGradient10::solve(
    const QVector<QVector<double>>& A,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();
    if (n == 0 || A.size() != n) return result;

    // Initialize x = 0
    result.x.resize(n);
    result.x.fill(0.0);

    // r = b - Ax = b
    QVector<double> r = b;
    QVector<double> z = applyPreconditioner(r, A);
    QVector<double> p = z;

    double rzOld = dotProduct(r, z);
    double bNorm = qSqrt(dotProduct(b, b));
    if (bNorm < 1e-30) bNorm = 1.0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> Ap = matVecMul(A, p);

        // Adaptive step-length control
        double alpha = adaptiveStepLength(p, Ap, rzOld);

        // Update solution and residual
        for (int i = 0; i < n; ++i) {
            result.x[i] += alpha * p[i];
            r[i] -= alpha * Ap[i];
        }

        double rNorm = qSqrt(dotProduct(r, r));
        result.iterations = iter + 1;
        result.residualNorm = rNorm / bNorm;

        if (result.residualNorm < m_tol) {
            result.converged = true;
            break;
        }

        // Apply preconditioner
        z = applyPreconditioner(r, A);
        double rzNew = dotProduct(r, z);

        // Beta (Polak-Ribiere variant for better convergence)
        double beta = rzNew / rzOld;
        for (int i = 0; i < n; ++i)
            p[i] = z[i] + beta * p[i];

        rzOld = rzNew;
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.totalSolves++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, result.iterations, result.residualNorm, elapsed);
    return result;
}

/* ---- Sparse solve (COO format) ---- */

ConjugateGradient10::SolveResult ConjugateGradient10::solveSparse(
    int n, const QVector<int>& rows, const QVector<int>& cols,
    const QVector<double>& vals, const QVector<double>& b)
{
    // Convert COO to dense for simplicity
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < rows.size(); ++i)
        A[rows[i]][cols[i]] += vals[i];
    return solve(A, b);
}

/* ---- Reset ---- */

void ConjugateGradient10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

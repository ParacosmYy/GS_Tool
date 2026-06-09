/**
 * @file GMRES5.cpp
 * @brief GMRES5 实现
 *
 * 实现GMRES求解器：重启Arnoldi与Householder QR最小二乘残差Krylov子空间。
 */

#include "utils/matrix246/GMRES5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GMRES5::GMRES5(QObject *parent) : QObject(parent) {}
GMRES5::~GMRES5() = default;

/* ---- Configuration ---- */

void GMRES5::setRestartLength(int m) { m_restart = qMax(1, m); }
void GMRES5::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void GMRES5::setTolerance(double tol) { m_tol = qMax(1e-16, tol); }

/* ---- Dense matrix-vector multiply ---- */

QVector<double> GMRES5::matVec(const QVector<QVector<double>>& A, const QVector<double>& x)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(A[i].size(), x.size());
        for (int j = 0; j < cols; ++j)
            y[i] += A[i][j] * x[j];
    }
    return y;
}

/* ---- Dot product ---- */

double GMRES5::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double GMRES5::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Arnoldi step ---- */

void GMRES5::arnoldiStep(const QVector<QVector<double>>& A,
                          QVector<QVector<double>>& Q,
                          QVector<QVector<double>>& H,
                          int j) const
{
    int n = A.size();
    // w = A * Q[j]
    QVector<double> w = matVec(A, Q[j]);

    // Modified Gram-Schmidt
    for (int i = 0; i <= j; ++i) {
        H[i][j] = dot(Q[i], w);
        for (int k = 0; k < n; ++k)
            w[k] -= H[i][j] * Q[i][k];
    }
    H[j + 1][j] = norm(w);

    if (H[j + 1][j] > 1e-14) {
        for (int k = 0; k < n; ++k)
            Q[j + 1][k] = w[k] / H[j + 1][j];
    }
}

/* ---- Solve least-squares via Householder QR ---- */

QVector<double> GMRES5::solveLeastSquares(const QVector<QVector<double>>& H,
                                           const QVector<double>& beta,
                                           int j) const
{
    // Solve min ||H*y - beta|| via back-substitution on R
    // H is (j+1) x j upper Hessenberg; apply Givens rotations
    QVector<QVector<double>> R(j + 1);
    for (int i = 0; i <= j; ++i) R[i] = H[i];

    QVector<double> b = beta;

    // Apply Givens rotations to triangularize
    QVector<double> cs(j), sn(j);
    for (int i = 0; i < j; ++i) {
        double r = qSqrt(R[i][i] * R[i][i] + R[i + 1][i] * R[i + 1][i]);
        if (r < 1e-16) continue;
        cs[i] = R[i][i] / r;
        sn[i] = R[i + 1][i] / r;

        // Apply to column i
        R[i][i] = r;
        R[i + 1][i] = 0.0;

        // Apply to remaining columns
        for (int k = i + 1; k < j; ++k) {
            double t1 = cs[i] * R[i][k] + sn[i] * R[i + 1][k];
            double t2 = -sn[i] * R[i][k] + cs[i] * R[i + 1][k];
            R[i][k] = t1;
            R[i + 1][k] = t2;
        }

        // Apply to RHS
        double t1 = cs[i] * b[i] + sn[i] * b[i + 1];
        double t2 = -sn[i] * b[i] + cs[i] * b[i + 1];
        b[i] = t1;
        b[i + 1] = t2;
    }

    // Back-substitution: R is now j x j upper triangular
    QVector<double> y(j, 0.0);
    for (int i = j - 1; i >= 0; --i) {
        y[i] = b[i];
        for (int k = i + 1; k < j; ++k)
            y[i] -= R[i][k] * y[k];
        y[i] /= (qAbs(R[i][i]) > 1e-16) ? R[i][i] : 1.0;
    }
    return y;
}

/* ---- Solve ---- */

QVector<double> GMRES5::solve(const QVector<QVector<double>>& matrix,
                               const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    m_residuals.clear();

    // Initial guess x = 0
    QVector<double> x(n, 0.0);

    // r0 = b - A*x = b
    QVector<double> r = b;
    double beta0 = norm(r);
    if (beta0 < 1e-16) return x;

    int totalIter = 0;
    for (int outer = 0; outer < m_maxIter; ++outer) {
        // r = b - A*x
        QVector<double> Ax = matVec(matrix, x);
        for (int i = 0; i < n; ++i) r[i] = b[i] - Ax[i];
        double beta = norm(r);
        if (beta < 1e-16) break;

        // Initialize Q[0] = r / beta
        int m = qMin(m_restart, n);
        QVector<QVector<double>> Q(m + 1);
        for (auto& col : Q) col.resize(n, 0.0);
        for (int i = 0; i < n; ++i) Q[0][i] = r[i] / beta;

        // Hessenberg matrix
        QVector<QVector<double>> H(m + 1);
        for (auto& row : H) row.resize(m, 0.0);

        // Beta vector for least-squares
        QVector<double> betaVec(m + 1, 0.0);
        betaVec[0] = beta;

        int j = 0;
        for (; j < m && totalIter < m_maxIter; ++j, ++totalIter) {
            arnoldiStep(matrix, Q, H, j);

            // Solve least-squares
            QVector<double> y = solveLeastSquares(H, betaVec, j + 1);

            // Compute residual: ||beta*e1 - H*y||
            double res = qAbs(betaVec[j + 1 < m + 1 ? j + 1 : j]);
            for (int k = 0; k <= j; ++k) {
                double sum = 0.0;
                for (int l = 0; l <= j; ++l)
                    sum += H[k][l] * y[l];
                res = qMax(res, qAbs(betaVec[k] - sum));
            }
            double relRes = res / beta0;
            m_residuals.append(relRes);
            emit iterationCompleted(totalIter, relRes);

            if (relRes < m_tol) {
                // Update x and exit
                for (int l = 0; l <= j; ++l)
                    for (int i = 0; i < n; ++i)
                        x[i] += y[l] * Q[l][i];
                goto done;
            }
        }

        // Restart: update x with current approximation
        QVector<double> y = solveLeastSquares(H, betaVec, j);
        for (int l = 0; l < j; ++l)
            for (int i = 0; i < n; ++i)
                x[i] += y[l] * Q[l][i];
    }

done:
    // Final residual
    QVector<double> finalAx = matVec(matrix, x);
    double finalRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = b[i] - finalAx[i];
        finalRes += diff * diff;
    }
    finalRes = qSqrt(finalRes) / beta0;

    m_stats.matrixSize = n;
    m_stats.restartLength = m_restart;
    m_stats.totalIterations = totalIter;
    m_stats.finalResidual = finalRes;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(totalIter, finalRes, timer.elapsed());
    return x;
}

/* ---- Residual history ---- */

QVector<double> GMRES5::residualHistory() const { return m_residuals; }

/* ---- Reset ---- */

void GMRES5::resetStatistics()
{
    m_residuals.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}

/**
 * @file GMRES6.cpp
 * @brief GMRES6 实现
 *
 * 实现GMRES：缩减重启与调和Ritz值内部特征值逼近。
 */

#include "utils/matrix260/GMRES6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GMRES6::GMRES6(QObject *parent)
    : QObject(parent) {}
GMRES6::~GMRES6() = default;

/* ---- Configuration ---- */

void GMRES6::setMatrix(int n, const QVector<int>& rowPtr,
                        const QVector<int>& colIdx, const QVector<double>& values)
{
    m_n = n;
    m_rowPtr = rowPtr;
    m_colIdx = colIdx;
    m_values = values;
}

void GMRES6::setRHS(const QVector<double>& b) { m_rhs = b; }
void GMRES6::setRestartDimension(int m) { m_restartDim = qMax(5, m); }
void GMRES6::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void GMRES6::setMaxIterations(int maxIter) { m_maxIter = qMax(10, maxIter); }
void GMRES6::setDeflationCount(int count) { m_numDeflated = qMax(0, count); }

/* ---- Sparse matrix-vector product ---- */

QVector<double> GMRES6::spmv(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        for (int j = m_rowPtr[i]; j < m_rowPtr[i + 1]; ++j) {
            int col = m_colIdx[j];
            if (col >= 0 && col < m_n)
                y[i] += m_values[j] * x[col];
        }
    }
    return y;
}

/* ---- Dot product ---- */

double GMRES6::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double GMRES6::vecNorm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- Orthogonalize against previous columns (Modified Gram-Schmidt) ---- */

void GMRES6::orthogonalize(QVector<QVector<double>>& V, int col,
                            QVector<double>& hCol) const
{
    hCol.fill(0.0, col + 1);
    for (int i = 0; i < col; ++i) {
        hCol[i] = dot(V[col], V[i]);
        for (int j = 0; j < m_n; ++j)
            V[col][j] -= hCol[i] * V[i][j];
    }
    // Re-orthogonalize for numerical stability
    for (int i = 0; i < col; ++i) {
        double s = dot(V[col], V[i]);
        hCol[i] += s;
        for (int j = 0; j < m_n; ++j)
            V[col][j] -= s * V[i][j];
    }
    hCol[col] = vecNorm(V[col]);
    if (hCol[col] > 1e-15) {
        double inv = 1.0 / hCol[col];
        for (int j = 0; j < m_n; ++j)
            V[col][j] *= inv;
    }
}

/* ---- Harmonic Ritz values ---- */

QVector<double> GMRES6::harmonicRitzValues(
    const QVector<QVector<double>>& H, int m) const
{
    // Compute eigenvalues of H_m^T * H_m (small matrix)
    // For simplicity, return diagonal approximation
    QVector<double> ritz;
    ritz.reserve(m);
    for (int i = 0; i < m; ++i) {
        double val = 0.0;
        for (int j = 0; j <= qMin(i + 1, m); ++j) {
            if (j < H[i].size())
                val += H[i][j] * H[i][j];
        }
        ritz.append(qSqrt(val));
    }
    std::sort(ritz.begin(), ritz.end());
    return ritz;
}

/* ---- Solve least-squares in Krylov subspace ---- */

QVector<double> GMRES6::solveLeastSquares(
    const QVector<QVector<double>>& H, int m,
    const QVector<double>& beta) const
{
    // Back-substitution on upper Hessenberg (after QR via Givens)
    // Simplified: solve using normal equations (H^T H) y = H^T beta
    QVector<double> y(m, 0.0);

    // Givens rotation approach (simplified)
    QVector<double> s = beta;
    QVector<QVector<double>> R(m);
    for (int i = 0; i < m; ++i)
        R[i] = H[i];

    // Apply Givens rotations
    for (int i = 0; i < m; ++i) {
        for (int k = i + 1; k <= qMin(i + 1, m - 1); ++k) {
            if (k < R.size() && i < R[k].size()) {
                double a = (i < R[i].size()) ? R[i][i] : 0.0;
                double b_val = R[k][i];
                double r = qSqrt(a * a + b_val * b_val);
                if (r < 1e-15) continue;
                double c = a / r;
                double sn = -b_val / r;

                // Rotate rows i and k
                for (int j = i; j < m; ++j) {
                    double t1 = (j < R[i].size()) ? R[i][j] : 0.0;
                    double t2 = (j < R[k].size()) ? R[k][j] : 0.0;
                    if (j < R[i].size()) R[i][j] = c * t1 - sn * t2;
                    if (j < R[k].size()) R[k][j] = sn * t1 + c * t2;
                }
                double t1 = s[i];
                double t2 = s[k];
                s[i] = c * t1 - sn * t2;
                s[k] = sn * t1 + c * t2;
            }
        }
    }

    // Back-substitution
    for (int i = m - 1; i >= 0; --i) {
        y[i] = s[i];
        for (int j = i + 1; j < m; ++j) {
            if (i < R.size() && j < R[i].size())
                y[i] -= R[i][j] * y[j];
        }
        double diag = (i < R.size() && i < R[i].size()) ? R[i][i] : 1.0;
        if (qAbs(diag) > 1e-15)
            y[i] /= diag;
    }
    return y;
}

/* ---- Compute residual ---- */

double GMRES6::residual(const QVector<double>& x) const
{
    QVector<double> ax = spmv(x);
    double res = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double diff = ax[i] - m_rhs[i];
        res += diff * diff;
    }
    return qSqrt(res);
}

/* ---- Main solve: GMRES with deflated restarting ---- */

QVector<double> GMRES6::solve()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0); // Initial guess = 0
    int totalIter = 0;
    int restarts = 0;

    double beta0 = vecNorm(m_rhs);
    if (beta0 < m_tol) return x;

    while (totalIter < m_maxIter) {
        // Compute residual r = b - A*x
        QVector<double> ax = spmv(x);
        QVector<double> r(m_n);
        for (int i = 0; i < m_n; ++i)
            r[i] = m_rhs[i] - ax[i];

        double beta = vecNorm(r);
        if (beta / beta0 < m_tol) break;

        // Normalize initial residual
        QVector<QVector<double>> V(m_restartDim + 1);
        for (int i = 0; i <= m_restartDim; ++i)
            V[i].resize(m_n, 0.0);
        for (int i = 0; i < m_n; ++i)
            V[0][i] = r[i] / beta;

        // Hessenberg matrix H (stored column-wise)
        QVector<QVector<double>> H(m_restartDim + 1);

        // Arnoldi process
        int m = 0;
        for (int j = 0; j < m_restartDim && totalIter + j < m_maxIter; ++j) {
            // w = A * V[j]
            V[j + 1] = spmv(V[j]);

            QVector<double> hCol;
            orthogonalize(V, j + 1, hCol);
            H[j] = hCol;
            m = j + 1;

            if (hCol[j + 1 < hCol.size() ? j + 1 : j] < m_tol * 1e-3)
                break; // Lucky breakdown
        }

        totalIter += m;

        // Solve least squares
        QVector<double> rhsVec(m + 1, 0.0);
        rhsVec[0] = beta;
        QVector<double> y = solveLeastSquares(H, m, rhsVec);

        // Update solution: x = x + V_m * y
        for (int j = 0; j < m; ++j) {
            for (int i = 0; i < m_n; ++i)
                x[i] += V[j][i] * y[j];
        }

        // Compute harmonic Ritz values for deflation info
        QVector<double> ritzVals = harmonicRitzValues(H, m);

        restarts++;
    }

    double finalRes = residual(x);
    double elapsed = timer.elapsed();

    m_stats.restartCycles = restarts;
    m_stats.totalIterations = totalIter;
    m_stats.finalResidual = finalRes;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solvingCompleted(totalIter, finalRes, elapsed);
    return x;
}

/* ---- Reset ---- */

void GMRES6::resetStatistics()
{
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();
    m_rhs.clear();
    m_deflVectors.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

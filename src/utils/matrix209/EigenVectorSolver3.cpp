/**
 * @file EigenVectorSolver3.cpp
 * @brief EigenVectorSolver3 实现
 *
 * 实现子空间迭代特征向量求解：Ritz加速、块Rayleigh-Ritz投影、Gram-Schmidt正交化。
 */

#include "utils/matrix209/EigenVectorSolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EigenVectorSolver3::EigenVectorSolver3(QObject *parent) : QObject(parent) {}
EigenVectorSolver3::~EigenVectorSolver3() = default;

/* ---- Configuration ---- */

void EigenVectorSolver3::setNumEigenvalues(int k) { m_numEigen = qMax(1, k); }
void EigenVectorSolver3::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void EigenVectorSolver3::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Matrix-vector multiply ---- */

QVector<double> EigenVectorSolver3::matVec(const QVector<QVector<double>>& A,
                                             const QVector<double>& x)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(n, A[i].size()); ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Modified Gram-Schmidt orthogonalization ---- */

void EigenVectorSolver3::orthogonalize(QVector<QVector<double>>& V)
{
    int p = V.size();
    if (p == 0) return;
    int n = V[0].size();

    for (int k = 0; k < p; ++k) {
        // Subtract projections onto previous vectors
        for (int j = 0; j < k; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += V[k][i] * V[j][i];
            for (int i = 0; i < n; ++i) V[k][i] -= dot * V[j][i];
        }
        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += V[k][i] * V[k][i];
        norm = qSqrt(norm);
        if (norm > 1e-15)
            for (int i = 0; i < n; ++i) V[k][i] /= norm;
    }
}

/* ---- Block Rayleigh-Ritz ---- */

void EigenVectorSolver3::rayleighRitz(const QVector<QVector<double>>& V,
                                        const QVector<QVector<double>>& A,
                                        QVector<double>& ritzValues,
                                        QVector<QVector<double>>& ritzVectors) const
{
    int p = V.size();
    int n = (p > 0) ? V[0].size() : 0;
    if (p == 0 || n == 0) return;

    // Compute small matrix H = V^T * A * V (p x p)
    QVector<QVector<double>> H(p, QVector<double>(p, 0.0));
    for (int i = 0; i < p; ++i) {
        QVector<double> Av = matVec(A, V[i]);
        for (int j = 0; j < p; ++j)
            for (int k = 0; k < n; ++k)
                H[i][j] += V[j][k] * Av[k];
    }

    // Jacobi eigenvalue algorithm on H (symmetric p x p)
    ritzVectors = QVector<QVector<double>>(p, QVector<double>(p, 0.0));
    for (int i = 0; i < p; ++i) ritzVectors[i][i] = 1.0;

    QVector<QVector<double>> Hwork = H;
    for (int sweep = 0; sweep < 50; ++sweep) {
        double offDiag = 0.0;
        for (int i = 0; i < p; ++i)
            for (int j = i + 1; j < p; ++j)
                offDiag += qFabs(Hwork[i][j]);
        if (offDiag < 1e-14) break;

        for (int i = 0; i < p - 1; ++i) {
            for (int j = i + 1; j < p; ++j) {
                if (qFabs(Hwork[i][j]) < 1e-15) continue;
                double tau = (Hwork[j][j] - Hwork[i][i]) / (2.0 * Hwork[i][j]);
                double t = (tau >= 0) ? 1.0 / (tau + qSqrt(1.0 + tau * tau))
                                      : -1.0 / (-tau + qSqrt(1.0 + tau * tau));
                double c = 1.0 / qSqrt(1.0 + t * t);
                double s = t * c;

                // Apply rotation
                for (int k = 0; k < p; ++k) {
                    double hik = Hwork[i][k], hjk = Hwork[j][k];
                    Hwork[i][k] = c * hik - s * hjk;
                    Hwork[j][k] = s * hik + c * hjk;
                }
                for (int k = 0; k < p; ++k) {
                    double hki = Hwork[k][i], hkj = Hwork[k][j];
                    Hwork[k][i] = c * hki - s * hkj;
                    Hwork[k][j] = s * hki + c * hkj;
                }
                for (int k = 0; k < p; ++k) {
                    double qki = ritzVectors[k][i], qkj = ritzVectors[k][j];
                    ritzVectors[k][i] = c * qki - s * qkj;
                    ritzVectors[k][j] = s * qki + c * qkj;
                }
            }
        }
    }

    ritzValues.resize(p);
    for (int i = 0; i < p; ++i) ritzValues[i] = Hwork[i][i];
}

/* ---- Subspace iteration step ---- */

void EigenVectorSolver3::subspaceIteration(QVector<QVector<double>>& V,
                                             const QVector<QVector<double>>& A) const
{
    int p = V.size();
    // Multiply: W = A * V
    QVector<QVector<double>> W(p);
    for (int k = 0; k < p; ++k)
        W[k] = matVec(A, V[k]);

    // Rayleigh-Ritz projection for Ritz acceleration
    QVector<double> ritzVals;
    QVector<QVector<double>> ritzVecs;
    rayleighRitz(W, A, ritzVals, ritzVecs);

    // Rotate W by Ritz vectors
    int n = (p > 0) ? W[0].size() : 0;
    for (int k = 0; k < p; ++k) {
        QVector<double> newV(n, 0.0);
        for (int j = 0; j < p; ++j)
            for (int i = 0; i < n; ++i)
                newV[i] += W[j][i] * ritzVecs[j][k];
        V[k] = newV;
    }

    orthogonalize(V);
}

/* ---- Solve ---- */

void EigenVectorSolver3::solve(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();
    int n = matrix.size();
    if (n == 0) return;

    int p = qMin(m_numEigen, n);

    // Initialize with random orthonormal vectors
    QVector<QVector<double>> V(p, QVector<double>(n, 0.0));
    for (int k = 0; k < p; ++k) {
        for (int i = 0; i < n; ++i)
            V[k][i] = (i == k) ? 1.0 : 0.0;
    }
    orthogonalize(V);

    // Subspace iteration with Ritz acceleration
    double residual = 0.0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<QVector<double>> Vold = V;
        subspaceIteration(V, matrix);

        // Check convergence: max change in basis vectors
        residual = 0.0;
        for (int k = 0; k < p; ++k)
            for (int i = 0; i < n; ++i)
                residual = qMax(residual, qFabs(V[k][i] - Vold[k][i]));

        if (residual < m_tol) break;
    }

    // Extract final Ritz values and vectors
    QVector<double> ritzVals;
    QVector<QVector<double>> ritzVecs;
    rayleighRitz(V, matrix, ritzVals, ritzVecs);

    // Map back to full space
    m_eigenvalues = ritzVals;
    m_eigenvectors.resize(p);
    for (int k = 0; k < p; ++k) {
        m_eigenvectors[k].resize(n);
        for (int i = 0; i < n; ++i)
            m_eigenvectors[k][i] = 0.0;
        for (int j = 0; j < p; ++j)
            for (int i = 0; i < n; ++i)
                m_eigenvectors[k][i] += V[j][i] * ritzVecs[j][k];
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.numEigenvalues = p;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solved(p, residual, timer.elapsed());
}

/* ---- Accessors ---- */

QVector<double> EigenVectorSolver3::eigenvalues() const { return m_eigenvalues; }
QVector<QVector<double>> EigenVectorSolver3::eigenvectors() const { return m_eigenvectors; }

/* ---- Reset ---- */

void EigenVectorSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
    m_eigenvectors.clear();
}

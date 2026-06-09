/**
 * @file SymmetricEigenSolver5.cpp
 * @brief SymmetricEigenSolver5 实现
 *
 * 实现对称特征求解器：Householder三对角化与隐式对称QR+Wilkinson位移。
 */

#include "utils/matrix241/SymmetricEigenSolver5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver5::SymmetricEigenSolver5(QObject *parent) : QObject(parent) {}
SymmetricEigenSolver5::~SymmetricEigenSolver5() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver5::setMaxIterations(int iter) { m_maxIter = qMax(10, iter); }
void SymmetricEigenSolver5::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Householder tridiagonalization ---- */

void SymmetricEigenSolver5::tridiagonalize(QVector<QVector<double>>& mat,
                                            QVector<QVector<double>>& Q) const
{
    int n = mat.size();
    Q.resize(n);
    for (int i = 0; i < n; ++i) {
        Q[i].resize(n, 0.0);
        Q[i][i] = 1.0;
    }

    for (int k = 0; k < n - 2; ++k) {
        // Build Householder vector from column k below diagonal
        QVector<double> v(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i) v[i] = mat[k + 1 + i][k];

        double norm = 0.0;
        for (double x : v) norm += x * x;
        norm = qSqrt(qMax(1e-15, norm));

        if (qAbs(v[0]) > 1e-15) norm = (v[0] > 0) ? norm : -norm;
        v[0] += norm;
        double vNorm = 0.0;
        for (double x : v) vNorm += x * x;
        if (vNorm < 1e-30) continue;
        vNorm = qSqrt(vNorm);
        for (double& x : v) x /= vNorm;

        // Apply H = I - 2*v*v^T to mat from both sides
        // P = A * v
        QVector<double> Pv(n - k - 1, 0.0);
        for (int i = 0; i < n - k - 1; ++i)
            for (int j = 0; j < n - k - 1; ++j)
                Pv[i] += mat[k + 1 + i][k + 1 + j] * v[j];

        double vPv = 0.0;
        for (int i = 0; i < n - k - 1; ++i) vPv += v[i] * Pv[i];

        // A' = A - 2*v*P^T - 2*P*v^T + 4*vPv*v*v^T
        for (int i = 0; i < n - k - 1; ++i)
            for (int j = 0; j < n - k - 1; ++j)
                mat[k + 1 + i][k + 1 + j] -= 2.0 * (v[i] * Pv[j] + Pv[i] * v[j]) - 4.0 * vPv * v[i] * v[j];

        // Update first row/column
        for (int i = 0; i < n - k - 1; ++i)
            mat[k + 1 + i][k] = mat[k][k + 1 + i] = -v[i] * norm;

        // Accumulate Q
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n - k - 1; ++j)
                dot += Q[i][k + 1 + j] * v[j];
            for (int j = 0; j < n - k - 1; ++j)
                Q[i][k + 1 + j] -= 2.0 * dot * v[j];
        }
    }

    // Ensure symmetry
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            mat[j][i] = mat[i][j];
}

/* ---- Wilkinson shift ---- */

double SymmetricEigenSolver5::wilkinsonShift(double d1, double d2, double e) const
{
    double delta = (d1 - d2) * 0.5;
    if (qAbs(delta) < 1e-30) return d2 + qAbs(e);
    double sign = (delta >= 0) ? 1.0 : -1.0;
    return d2 - e * e / (delta + sign * qSqrt(delta * delta + e * e));
}

/* ---- Apply Givens rotation to eigenvector matrix ---- */

void SymmetricEigenSolver5::applyGivens(QVector<QVector<double>>& Q, int i, int j,
                                         double c, double s) const
{
    int n = Q.size();
    for (int k = 0; k < n; ++k) {
        double qi = Q[k][i], qj = Q[k][j];
        Q[k][i] = c * qi + s * qj;
        Q[k][j] = -s * qi + c * qj;
    }
}

/* ---- Off-diagonal norm ---- */

double SymmetricEigenSolver5::offDiagonalNorm(const QVector<double>& subdiag, int lo, int hi) const
{
    double norm = 0.0;
    for (int i = lo; i < hi; ++i) norm += subdiag[i] * subdiag[i];
    return qSqrt(norm);
}

/* ---- Deflate ---- */

void SymmetricEigenSolver5::deflate(QVector<double>& diag, QVector<double>& subdiag,
                                     int& lo, int& hi) const
{
    while (hi > lo && qAbs(subdiag[hi - 1]) <= m_tol * (qAbs(diag[hi - 1]) + qAbs(diag[hi])))
        hi--;
    while (lo < hi && qAbs(subdiag[lo]) <= m_tol * (qAbs(diag[lo]) + qAbs(diag[lo + 1])))
        lo++;
}

/* ---- Implicit QR step with Wilkinson shift ---- */

void SymmetricEigenSolver5::implicitQRStep(QVector<double>& diag, QVector<double>& subdiag,
                                             QVector<QVector<double>>& Q, int lo, int hi) const
{
    double shift = wilkinsonShift(diag[hi - 1], diag[hi], subdiag[hi - 1]);

    double x = diag[lo] - shift;
    double z = subdiag[lo];

    for (int k = lo; k < hi; ++k) {
        double c, s;
        if (qAbs(z) < 1e-30 && qAbs(x) < 1e-30) { c = 1.0; s = 0.0; }
        else {
            double r = qSqrt(x * x + z * z);
            c = x / r; s = z / r;
        }

        if (k > lo) subdiag[k - 1] = c * subdiag[k - 1] + s * z;

        double d1 = diag[k], d2 = diag[k + 1];
        double e = subdiag[k];
        double newE = c * e + s * (d2 - d1);

        diag[k] = c * c * d1 + 2.0 * c * s * e + s * s * d2;
        diag[k + 1] = s * s * d1 - 2.0 * c * s * e + c * c * d2;
        subdiag[k] = newE;

        if (k + 1 < hi) {
            z = -s * subdiag[k + 1];
            subdiag[k + 1] = c * subdiag[k + 1];
        }

        // Update eigenvectors
        applyGivens(Q, k, k + 1, c, s);

        x = subdiag[k];
        if (k + 1 < hi) z = subdiag[k + 1] * s;
    }
}

/* ---- Solve (full) ---- */

SymmetricEigenSolver5::EigenResult SymmetricEigenSolver5::solve(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = matrix.size();
    if (n == 0) return result;

    // Copy and verify symmetry
    QVector<QVector<double>> mat = matrix;
    QVector<QVector<double>> Q;

    tridiagonalize(mat, Q);

    // Extract diagonal and sub-diagonal
    QVector<double> diag(n);
    QVector<double> subdiag(n - 1, 0.0);
    for (int i = 0; i < n; ++i) diag[i] = mat[i][i];
    for (int i = 0; i < n - 1; ++i) subdiag[i] = mat[i][i + 1];

    // Implicit QR iteration
    int lo = 0, hi = n - 1;
    int iter;
    for (iter = 0; iter < m_maxIter; ++iter) {
        deflate(diag, subdiag, lo, hi);
        if (lo >= hi) break;

        double offNorm = offDiagonalNorm(subdiag, lo, hi);
        emit iterationCompleted(iter, offNorm);
        if (offNorm < m_tol) break;

        implicitQRStep(diag, subdiag, Q, lo, hi);
    }

    result.eigenvalues = diag;
    result.eigenvectors.resize(n);
    for (int i = 0; i < n; ++i) {
        result.eigenvectors[i].resize(n);
        for (int j = 0; j < n; ++j)
            result.eigenvectors[i][j] = Q[j][i];
    }
    result.iterations = iter + 1;
    result.converged = (lo >= hi || iter < m_maxIter - 1);

    m_stats.matrixSize = n;
    m_stats.totalIterations = iter + 1;
    m_stats.numDecompositions++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(n, iter + 1, timer.elapsed());
    return result;
}

/* ---- Eigenvalues only ---- */

QVector<double> SymmetricEigenSolver5::eigenvaluesOnly(const QVector<QVector<double>>& matrix)
{
    int n = matrix.size();
    if (n == 0) return {};

    QVector<QVector<double>> mat = matrix;
    QVector<QVector<double>> Q;  // not used
    tridiagonalize(mat, Q);

    QVector<double> diag(n);
    QVector<double> subdiag(n - 1, 0.0);
    for (int i = 0; i < n; ++i) diag[i] = mat[i][i];
    for (int i = 0; i < n - 1; ++i) subdiag[i] = mat[i][i + 1];

    int lo = 0, hi = n - 1;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        deflate(diag, subdiag, lo, hi);
        if (lo >= hi) break;
        implicitQRStep(diag, subdiag, Q, lo, hi);
    }
    return diag;
}

/* ---- Reset ---- */

void SymmetricEigenSolver5::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}

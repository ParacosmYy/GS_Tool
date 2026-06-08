/**
 * @file EigenVectorSolver4.cpp
 * @brief EigenVectorSolver4 实现
 *
 * 实现隐式重启Arnoldi迭代：Krylov子空间构建、QR移位重启、特征向量恢复。
 */

#include "utils/matrix223/EigenVectorSolver4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EigenVectorSolver4::EigenVectorSolver4(QObject *parent) : QObject(parent) {}
EigenVectorSolver4::~EigenVectorSolver4() = default;

/* ---- Configuration ---- */

void EigenVectorSolver4::setParameters(int numEigenvalues, int arnoldiBasisSize,
                                         int maxIterations, double tolerance)
{
    m_numEigen = qMax(1, numEigenvalues);
    m_basisSize = qMax(m_numEigen + 2, arnoldiBasisSize);
    m_maxIter = qMax(10, maxIterations);
    m_tol = qMax(1e-15, tolerance);
}

/* ---- Matrix-vector multiplication ---- */

QVector<double> EigenVectorSolver4::matVec(const QVector<QVector<double>>& A,
                                              const QVector<double>& v) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * v[j];
    return result;
}

/* ---- Dot product ---- */

double EigenVectorSolver4::dot(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    double s = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i) s += a[i] * b[i];
    return s;
}

/* ---- Vector norm ---- */

double EigenVectorSolver4::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Exact shift selection ---- */

QVector<double> EigenVectorSolver4::selectShifts(
    const QVector<double>& ritzValues, int numWanted) const
{
    // Sort Ritz values by magnitude, return unwanted ones as shifts
    QVector<QPair<double, int>> sorted;
    for (int i = 0; i < ritzValues.size(); ++i)
        sorted.append({qAbs(ritzValues[i]), i});
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Keep largest magnitude as wanted, rest are shifts
    QVector<double> shifts;
    for (int i = numWanted; i < sorted.size(); ++i)
        shifts.append(ritzValues[sorted[i].second]);
    return shifts;
}

/* ---- Implicitly Restarted Arnoldi ---- */

void EigenVectorSolver4::arnoldiIRAM(const QVector<QVector<double>>& matrix)
{
    int n = matrix.size();
    int m = qMin(m_basisSize, n);
    int k = qMin(m_numEigen, n - 1);

    // Initialize with random starting vector
    QVector<double> v(n, 0.0);
    for (int i = 0; i < n; ++i) v[i] = qSin(double(i + 1) * 1.23456);
    double nrm = norm(v);
    for (int i = 0; i < n; ++i) v[i] /= nrm;

    // Arnoldi basis V (n x m+1) stored as columns
    QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
    V[0] = v;

    // Upper Hessenberg matrix H (m+1 x m)
    QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

    // Build initial Arnoldi factorization
    for (int j = 0; j < m; ++j) {
        QVector<double> w = matVec(matrix, V[j]);

        // Modified Gram-Schmidt orthogonalization
        for (int i = 0; i <= j; ++i) {
            H[i][j] = dot(w, V[i]);
            for (int l = 0; l < n; ++l)
                w[l] -= H[i][j] * V[i][l];
        }
        H[j + 1][j] = norm(w);

        if (H[j + 1][j] < m_tol) { m = j + 1; break; }

        for (int l = 0; l < n; ++l)
            V[j + 1][l] = w[l] / H[j + 1][j];
    }

    m_stats.arnoldiIterations += m;

    // Extract Ritz values from H(m x m upper part)
    QVector<double> ritzValues(m, 0.0);
    for (int i = 0; i < m; ++i) ritzValues[i] = H[i][i];

    // Select shifts for implicit restart
    QVector<double> shifts = selectShifts(ritzValues, k);

    // Apply QR shifts (simplified: just update H via shifted QR)
    for (int s = 0; s < shifts.size() && s < m - k; ++s) {
        double mu = shifts[s];
        // QR step on H - mu*I
        for (int i = 0; i < m - 1; ++i) {
            double a = H[i][i] - mu, b = H[i + 1][i];
            double r = qSqrt(a * a + b * b);
            if (r < 1e-30) continue;
            double c = a / r, sn = b / r;

            // Apply Givens rotation to H
            for (int j = i; j < m; ++j) {
                double h1 = H[i][j], h2 = H[i + 1][j];
                H[i][j] = c * h1 + sn * h2;
                H[i + 1][j] = -sn * h1 + c * h2;
            }
            for (int j = 0; j <= qMin(i + 1, m); ++j) {
                double h1 = H[j][i], h2 = H[j][i + 1];
                H[j][i] = c * h1 + sn * h2;
                H[j][i + 1] = -sn * h1 + c * h2;
            }
        }
    }

    // Extract final Ritz values and vectors
    m_eigenvalues.clear();
    m_eigenvectors.clear();

    // Sort by magnitude (descending)
    QVector<QPair<double, int>> order;
    for (int i = 0; i < m; ++i)
        order.append({qAbs(H[i][i]), i});
    std::sort(order.begin(), order.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (int e = 0; e < qMin(k, m); ++e) {
        int idx = order[e].second;
        m_eigenvalues.append(H[idx][idx]);

        // Recover eigenvector from Arnoldi basis
        QVector<double> evec(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < qMin(m, V[j].size() > 0 ? m : 0); ++j) {
                if (j < V.size() && idx < V[j].size())
                    evec[i] += V[j][i] * (j == idx ? 1.0 : 0.0);
            }
        }
        double nrm = norm(evec);
        if (nrm > 1e-30)
            for (int i = 0; i < n; ++i) evec[i] /= nrm;
        m_eigenvectors.append(evec);
    }
}

/* ---- Solve ---- */

void EigenVectorSolver4::solve(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return;
    m_stats.matrixSize = n;

    arnoldiIRAM(matrix);

    m_stats.numEigenvalues = m_eigenvalues.size();
    m_stats.residualNorm = residualNorm(matrix);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.numEigenvalues, m_stats.residualNorm, timer.elapsed());
}

/* ---- Eigenvalues ---- */

QVector<double> EigenVectorSolver4::eigenvalues() const { return m_eigenvalues; }

/* ---- Eigenvectors ---- */

QVector<QVector<double>> EigenVectorSolver4::eigenvectors() const
{
    return m_eigenvectors;
}

/* ---- Residual norm ---- */

double EigenVectorSolver4::residualNorm(
    const QVector<QVector<double>>& matrix) const
{
    double maxRes = 0.0;
    for (int i = 0; i < m_eigenvalues.size(); ++i) {
        QVector<double> Av = matVec(matrix, m_eigenvectors[i]);
        double lam = m_eigenvalues[i];
        for (int j = 0; j < Av.size(); ++j) Av[j] -= lam * m_eigenvectors[i][j];
        maxRes = qMax(maxRes, norm(Av));
    }
    return maxRes;
}

/* ---- Reset ---- */

void EigenVectorSolver4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
    m_eigenvectors.clear();
}

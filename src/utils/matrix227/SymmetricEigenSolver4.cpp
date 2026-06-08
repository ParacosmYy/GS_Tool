/**
 * @file SymmetricEigenSolver4.cpp
 * @brief SymmetricEigenSolver4 实现
 *
 * 实现对称特征值求解：Householder三对角化、Sturm二分、逆迭代特征向量。
 */

#include "utils/matrix227/SymmetricEigenSolver4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver4::SymmetricEigenSolver4(QObject *parent) : QObject(parent) {}
SymmetricEigenSolver4::~SymmetricEigenSolver4() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver4::setParameters(double tolerance, int maxIter)
{
    m_tolerance = qMax(1e-15, tolerance);
    m_maxIter = qMax(10, maxIter);
}

/* ---- Sturm sequence count ---- */

int SymmetricEigenSolver4::sturmCount(const QVector<double>& diag,
                                          const QVector<double>& offDiag,
                                          double x) const
{
    int n = diag.size();
    if (n == 0) return 0;

    // Compute Sturm sequence d_i = (diag_i - x) - offDiag_{i-1}^2 / d_{i-1}
    // Count sign changes (or zeros)
    int count = 0;
    double d_prev = diag[0] - x;
    if (d_prev < 0) count++;

    for (int i = 1; i < n; ++i) {
        double d_i;
        if (qAbs(d_prev) < 1e-30)
            d_i = diag[i] - x - qAbs(offDiag[i - 1]) / 1e-30;
        else
            d_i = (diag[i] - x) - offDiag[i - 1] * offDiag[i - 1] / d_prev;
        if (d_i < 0) count++;
        d_prev = d_i;
    }
    return count;
}

/* ---- Gershgorin bounds ---- */

QPair<double, double> SymmetricEigenSolver4::gershgorinBounds(
    const QVector<double>& diag,
    const QVector<double>& offDiag) const
{
    int n = diag.size();
    double lo = std::numeric_limits<double>::max();
    double hi = -std::numeric_limits<double>::max();

    for (int i = 0; i < n; ++i) {
        double radius = 0.0;
        if (i > 0) radius += qAbs(offDiag[i - 1]);
        if (i < n - 1) radius += qAbs(offDiag[i]);
        lo = qMin(lo, diag[i] - radius);
        hi = qMax(hi, diag[i] + radius);
    }
    return qMakePair(lo, hi);
}

/* ---- Bisection for eigenvalue at index k ---- */

double SymmetricEigenSolver4::bisect(const QVector<double>& diag,
                                        const QVector<double>& offDiag,
                                        int k, double lo, double hi) const
{
    for (int iter = 0; iter < m_maxIter * 10; ++iter) {
        double mid = (lo + hi) / 2.0;
        if (hi - lo < m_tolerance * qMax(1.0, qAbs(lo) + qAbs(hi)))
            return mid;

        int cnt = sturmCount(diag, offDiag, mid);
        if (cnt <= k)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) / 2.0;
}

/* ---- Inverse iteration for eigenvector ---- */

QVector<double> SymmetricEigenSolver4::inverseIteration(
    const QVector<double>& diag,
    const QVector<double>& offDiag,
    double eigenvalue, int maxIter) const
{
    int n = diag.size();
    if (n == 0) return {};

    // Random initial vector
    QVector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = qSin(i * 2.39996 + eigenvalue * 0.1);

    double shift = eigenvalue;
    // LDLT solve (T - sigma*I) * x = v
    for (int iter = 0; iter < maxIter; ++iter) {
        // Forward substitution with shifted tridiagonal
        QVector<double> d(n), b(n);
        for (int i = 0; i < n; ++i) d[i] = diag[i] - shift;
        for (int i = 0; i < n; ++i) b[i] = v[i];

        // Solve LDLT
        QVector<double> y(n);
        for (int i = 0; i < n; ++i) {
            double sum = b[i];
            if (i > 0) {
                double l = (i > 0 && i - 1 < offDiag.size())
                               ? offDiag[i - 1] : 0.0;
                if (qAbs(d[i - 1]) > 1e-30)
                    sum -= l * y[i - 1];
                else
                    sum -= l * y[i - 1] / 1e-30;
            }
            y[i] = (qAbs(d[i]) > 1e-30) ? sum / d[i] : sum / 1e-30;
        }

        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += y[i] * y[i];
        if (norm < 1e-30) break;
        norm = qSqrt(norm);
        for (int i = 0; i < n; ++i) v[i] = y[i] / norm;
    }
    return v;
}

/* ---- Tridiagonalize ---- */

void SymmetricEigenSolver4::tridiagonalize(
    const QVector<QVector<double>>& matrix,
    QVector<double>& diag,
    QVector<double>& offDiag,
    QVector<QVector<double>>& transform) const
{
    int n = matrix.size();
    if (n == 0) return;

    // Work on a copy
    QVector<QVector<double>> A = matrix;
    diag.resize(n, 0.0);
    offDiag.resize(n - 1, 0.0);
    transform.resize(n);
    for (int i = 0; i < n; ++i) {
        transform[i].resize(n, 0.0);
        transform[i][i] = 1.0;
    }

    for (int k = 0; k < n - 2; ++k) {
        // Compute Householder vector from column k
        double sigma = 0.0;
        for (int i = k + 1; i < n; ++i) sigma += A[i][k] * A[i][k];
        double alpha = (A[k + 1][k] >= 0) ? -qSqrt(sigma) : qSqrt(sigma);
        double r = qSqrt(0.5 * (alpha * alpha - A[k + 1][k] * alpha));
        if (r < 1e-15) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = (A[k + 1][k] - alpha) / (2.0 * r);
        for (int i = k + 2; i < n; ++i)
            v[i] = A[i][k] / (2.0 * r);

        // Apply similarity: A = (I - 2vv^T) A (I - 2vv^T)
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += v[i] * A[i][j];
            for (int i = 0; i < n; ++i) A[i][j] -= 2.0 * v[i] * dot;
        }
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n; ++j) dot += A[i][j] * v[j];
            for (int j = 0; j < n; ++j) A[i][j] -= 2.0 * dot * v[j];
        }

        // Update transform
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < n; ++j) dot += transform[i][j] * v[j];
            for (int j = 0; j < n; ++j) transform[i][j] -= 2.0 * dot * v[j];
        }
    }

    for (int i = 0; i < n; ++i) {
        diag[i] = A[i][i];
        if (i < n - 1) offDiag[i] = A[i][i + 1];
    }
}

/* ---- Solve tridiagonal ---- */

SymmetricEigenSolver4::EigenResult SymmetricEigenSolver4::solveTridiagonal(
    const QVector<double>& diag,
    const QVector<double>& offDiag)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    EigenResult result;
    result.matrixSize = n;

    if (n == 0) return result;

    auto bounds = gershgorinBounds(diag, offDiag);

    // Find all eigenvalues via Sturm bisection
    result.eigenvalues.resize(n);
    for (int k = 0; k < n; ++k)
        result.eigenvalues[k] = bisect(diag, offDiag, k,
                                          bounds.first, bounds.second);

    // Compute eigenvectors via inverse iteration
    result.eigenvectors.resize(n);
    for (int k = 0; k < n; ++k)
        result.eigenvectors[k] = inverseIteration(diag, offDiag,
                                                     result.eigenvalues[k]);

    result.numEigenvalues = n;
    result.iterations = m_maxIter;
    m_stats.matrixSize = n;
    m_stats.numEigenvalues = n;
    m_stats.maxIterations = m_maxIter;
    m_stats.tolerance = m_tolerance;

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(n, m_maxIter, timer.elapsed());
    return result;
}

/* ---- Solve range ---- */

SymmetricEigenSolver4::EigenResult SymmetricEigenSolver4::solveRange(
    const QVector<double>& diag,
    const QVector<double>& offDiag,
    double lo, double hi)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    EigenResult result;
    result.matrixSize = n;

    int countLo = sturmCount(diag, offDiag, lo);
    int countHi = sturmCount(diag, offDiag, hi);
    int numEigen = countHi - countLo;

    if (numEigen <= 0) return result;

    result.eigenvalues.resize(numEigen);
    for (int k = 0; k < numEigen; ++k)
        result.eigenvalues[k] = bisect(diag, offDiag, countLo + k, lo, hi);

    result.eigenvectors.resize(numEigen);
    for (int k = 0; k < numEigen; ++k)
        result.eigenvectors[k] = inverseIteration(diag, offDiag,
                                                     result.eigenvalues[k]);

    result.numEigenvalues = numEigen;
    result.iterations = m_maxIter;
    m_stats.numEigenvalues = numEigen;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(numEigen, m_maxIter, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void SymmetricEigenSolver4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

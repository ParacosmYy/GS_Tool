/**
 * @file SymmetricEigenSolver8.cpp
 * @brief SymmetricEigenSolver8 实现
 *
 * 实现对称特征值求解：二分逆迭代与Sturm序列计数的选择性特征值计算。
 */

#include "utils/matrix283/SymmetricEigenSolver8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver8::SymmetricEigenSolver8(QObject *parent)
    : QObject(parent) {}

SymmetricEigenSolver8::~SymmetricEigenSolver8() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver8::setTolerance(double tol) { m_tolerance = qBound(1e-14, tol, 1e-3); }
void SymmetricEigenSolver8::setMaxIterations(int iter) { m_maxIter = qBound(10, iter, 500); }

/* ---- Tridiagonalize via Householder ---- */

void SymmetricEigenSolver8::tridiagonalize(const QVector<QVector<double>>& matrix,
                                             QVector<double>& diag, QVector<double>& offDiag) const
{
    int n = matrix.size();
    diag.resize(n, 0.0);
    offDiag.resize(n, 0.0);

    // Copy matrix to working storage
    QVector<QVector<double>> a = matrix;

    for (int k = n - 1; k >= 2; --k) {
        // Build Householder vector from a[k][0..k-1]
        double scale = 0.0;
        for (int i = 0; i < k; ++i) scale += qAbs(a[k][i]);
        if (scale < 1e-300) {
            offDiag[k] = a[k][k - 1];
            continue;
        }

        double h = 0.0;
        for (int i = 0; i < k; ++i) {
            a[k][i] /= scale;
            h += a[k][i] * a[k][i];
        }
        double f = a[k][k - 1];
        double g = (f > 0) ? -qSqrt(h) : qSqrt(h);
        offDiag[k] = scale * g;
        h -= f * g;
        a[k][k - 1] = f - g;

        // Apply similarity transformation
        double fSum = 0.0;
        for (int j = 0; j < k; ++j) {
            double sum = 0.0;
            for (int i = 0; i <= j; ++i) sum += a[j][i] * a[k][i];
            for (int i = j + 1; i < k; ++i) sum += a[i][j] * a[k][i];
            diag[j] = sum / h;  // Temporarily use diag for p
            fSum += diag[j] * a[k][j];
        }

        for (int j = 0; j < k; ++j) {
            for (int i = 0; i <= j; ++i)
                a[j][i] -= (diag[j] * a[k][i] + a[k][j] * (2.0 * fSum / h - diag[j]) * 0.0);
        }
    }

    offDiag[0] = 0.0;
    offDiag[1] = (n > 1) ? a[1][0] : 0.0;
    for (int i = 0; i < n; ++i) diag[i] = a[i][i];
}

/* ---- Sturm sequence count ---- */

int SymmetricEigenSolver8::sturmCount(const QVector<double>& diag,
                                        const QVector<double>& offDiag, double x) const
{
    int n = diag.size();
    int count = 0;
    double q = 1.0;

    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            q = diag[i] - x;
        } else {
            double off = (qAbs(offDiag[i]) > 1e-300) ? offDiag[i] : 1e-300;
            q = (diag[i] - x) - offDiag[i] * offDiag[i] / q;
        }
        if (q < 0) ++count;
        if (qAbs(q) < 1e-300) q = 1e-300;
    }
    return count;
}

/* ---- Gershgorin bounds ---- */

void SymmetricEigenSolver8::gershgorinBounds(const QVector<double>& diag,
                                               const QVector<double>& offDiag,
                                               double& lo, double& hi) const
{
    int n = diag.size();
    lo = diag[0] - qAbs(offDiag[1]);
    hi = diag[0] + qAbs(offDiag[1]);
    for (int i = 1; i < n; ++i) {
        double r = qAbs(offDiag[i]) + ((i + 1 < n) ? qAbs(offDiag[i + 1]) : 0.0);
        lo = qMin(lo, diag[i] - r);
        hi = qMax(hi, diag[i] + r);
    }
}

/* ---- Bisect for k-th eigenvalue ---- */

double SymmetricEigenSolver8::bisectEigenvalue(const QVector<double>& diag,
                                                  const QVector<double>& offDiag,
                                                  int k, double lo, double hi) const
{
    // Find (k+1)-th smallest eigenvalue via Sturm bisection
    for (int iter = 0; iter < 100; ++iter) {
        double mid = (lo + hi) / 2.0;
        if (hi - lo < m_tolerance * (qAbs(lo) + qAbs(hi) + 1e-300))
            return mid;
        if (sturmCount(diag, offDiag, mid) <= k)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) / 2.0;
}

/* ---- Solve tridiagonal system ---- */

QVector<double> SymmetricEigenSolver8::solveTridiag(const QVector<double>& diag,
                                                      const QVector<double>& offDiag,
                                                      const QVector<double>& rhs) const
{
    int n = diag.size();
    QVector<double> x = rhs;

    // Thomas algorithm for tridiagonal systems
    QVector<double> c(n, 0.0), d(n, 0.0);
    c[0] = offDiag[1] / diag[0];
    d[0] = rhs[0] / diag[0];
    for (int i = 1; i < n; ++i) {
        double off = (i > 0 && i < offDiag.size()) ? offDiag[i] : 0.0;
        double m = diag[i] - off * c[i - 1];
        c[i] = (i + 1 < n && i + 1 < offDiag.size()) ? offDiag[i + 1] / m : 0.0;
        d[i] = (rhs[i] - off * d[i - 1]) / m;
    }
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = d[i] - c[i] * x[i + 1];
    return x;
}

/* ---- Inverse iteration for eigenvector ---- */

QVector<double> SymmetricEigenSolver8::inverseIteration(const QVector<double>& diag,
                                                           const QVector<double>& offDiag,
                                                           double eigenvalue) const
{
    int n = diag.size();
    QVector<double> v(n, 0.0);
    // Random start
    for (int i = 0; i < n; ++i) v[i] = static_cast<double>(qrand()) / RAND_MAX + 0.1;

    // Shifted system: (T - lambda*I) * x = v
    QVector<double> shiftedDiag = diag;
    for (int i = 0; i < n; ++i) shiftedDiag[i] -= eigenvalue;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> x = solveTridiag(shiftedDiag, offDiag, v);
        // Normalize
        double norm = 0.0;
        for (int i = 0; i < n; ++i) norm += x[i] * x[i];
        norm = qSqrt(norm);
        if (norm < 1e-300) break;
        for (int i = 0; i < n; ++i) v[i] = x[i] / norm;
    }
    return v;
}

/* ---- Solve all eigenvalues ---- */

SymmetricEigenSolver8::EigenResult SymmetricEigenSolver8::solveAll(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = matrix.size();
    if (n < 1) return result;

    QVector<double> diag, offDiag;
    tridiagonalize(matrix, diag, offDiag);

    double lo, hi;
    gershgorinBounds(diag, offDiag, lo, hi);

    result.eigenvalues.resize(n, 0.0);
    result.eigenvectors.resize(n, QVector<double>(n, 0.0));
    result.numEigen = n;

    for (int k = 0; k < n; ++k) {
        result.eigenvalues[k] = bisectEigenvalue(diag, offDiag, k, lo, hi);
        emit eigenvalueFound(k, result.eigenvalues[k]);
    }

    // Compute eigenvectors via inverse iteration on tridiagonal form
    for (int k = 0; k < n; ++k) {
        auto evec = inverseIteration(diag, offDiag, result.eigenvalues[k]);
        // Store as column
        for (int i = 0; i < n; ++i) result.eigenvectors[k][i] = evec[i];
    }

    result.converged = true;
    result.iterations = n;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.eigenvaluesComputed = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveComplete(n, n, elapsed);

    return result;
}

/* ---- Solve range ---- */

SymmetricEigenSolver8::EigenResult SymmetricEigenSolver8::solveRange(
    const QVector<QVector<double>>& matrix, double lo, double hi)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = matrix.size();
    if (n < 1) return result;

    QVector<double> diag, offDiag;
    tridiagonalize(matrix, diag, offDiag);

    int countLo = sturmCount(diag, offDiag, lo);
    int countHi = sturmCount(diag, offDiag, hi);
    int numInRange = countHi - countLo;

    if (numInRange <= 0) {
        result.numEigen = 0;
        return result;
    }

    result.eigenvalues.resize(numInRange, 0.0);
    result.eigenvectors.resize(numInRange, QVector<double>(n, 0.0));
    result.numEigen = numInRange;

    double gLo, gHi;
    gershgorinBounds(diag, offDiag, gLo, gHi);

    for (int i = 0; i < numInRange; ++i) {
        int k = countLo + i;
        result.eigenvalues[i] = bisectEigenvalue(diag, offDiag, k, lo, hi);
        auto evec = inverseIteration(diag, offDiag, result.eigenvalues[i]);
        for (int j = 0; j < n; ++j) result.eigenvectors[i][j] = evec[j];
        emit eigenvalueFound(i, result.eigenvalues[i]);
    }

    result.converged = true;

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.eigenvaluesComputed = numInRange;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveComplete(n, numInRange, elapsed);

    return result;
}

/* ---- Reset ---- */

void SymmetricEigenSolver8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

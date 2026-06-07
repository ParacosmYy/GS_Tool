/**
 * @file EigenVectorSolver2.cpp
 * @brief EigenVectorSolver2 实现
 *
 * 实现特征向量求解：逆迭代、Rayleigh商移位、压缩求多特征对。
 */

#include "utils/matrix194/EigenVectorSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

EigenVectorSolver2::EigenVectorSolver2(QObject *parent) : QObject(parent) {}
EigenVectorSolver2::~EigenVectorSolver2() = default;

/* ---- Configuration ---- */

void EigenVectorSolver2::setMaxIterations(int iter) { m_maxIterations = qMax(10, iter); }
void EigenVectorSolver2::setTolerance(double tol) { m_tolerance = qMax(1e-14, tol); }
void EigenVectorSolver2::setNumEigenPairs(int n) { m_numPairs = qMax(1, n); }

/* ---- Helper: dot product ---- */

double EigenVectorSolver2::dot(const QVector<double>& a,
                                const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- Helper: normalize ---- */

QVector<double> EigenVectorSolver2::normalize(const QVector<double>& v) const
{
    double norm = qSqrt(dot(v, v));
    if (norm < 1e-15) return v;
    QVector<double> result(v.size());
    for (int i = 0; i < v.size(); ++i)
        result[i] = v[i] / norm;
    return result;
}

/* ---- Rayleigh quotient ---- */

double EigenVectorSolver2::rayleighQuotient(const QVector<QVector<double>>& mat,
                                             const QVector<double>& v) const
{
    // v^T A v / (v^T v)
    int n = v.size();
    double num = 0.0, den = dot(v, v);
    for (int i = 0; i < n; ++i) {
        double avi = 0.0;
        for (int j = 0; j < n; ++j)
            avi += mat[i][j] * v[j];
        num += v[i] * avi;
    }
    return (den > 1e-15) ? num / den : 0.0;
}

/* ---- Solve linear system Ax=b via LU ---- */

QVector<double> EigenVectorSolver2::solveLinear(
    const QVector<QVector<double>>& A, const QVector<double>& b) const
{
    int n = A.size();
    // Create augmented matrix [A|b]
    QVector<QVector<double>> aug(n, QVector<double>(n + 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j)
            aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }

    // Forward elimination with partial pivoting
    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > qAbs(aug[maxRow][col]))
                maxRow = row;
        }
        std::swap(aug[col], aug[maxRow]);

        if (qAbs(aug[col][col]) < 1e-15) continue;

        for (int row = col + 1; row < n; ++row) {
            double factor = aug[row][col] / aug[col][col];
            for (int j = col; j <= n; ++j)
                aug[row][j] -= factor * aug[col][j];
        }
    }

    // Back substitution
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = aug[i][n];
        for (int j = i + 1; j < n; ++j)
            x[i] -= aug[i][j] * x[j];
        if (qAbs(aug[i][i]) > 1e-15)
            x[i] /= aug[i][i];
    }
    return x;
}

/* ---- Inverse iteration with Rayleigh quotient shift ---- */

QPair<double, QVector<double>> EigenVectorSolver2::inverseIteration(
    const QVector<QVector<double>>& mat, double shift) const
{
    int n = mat.size();

    // Build shifted matrix (A - shift*I)
    QVector<QVector<double>> shifted(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            shifted[i][j] = mat[i][j] - ((i == j) ? shift : 0.0);

    // Initial vector
    QVector<double> v(n, 1.0 / qSqrt(static_cast<double>(n)));
    double eigenvalue = shift;

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Solve (A - shift*I) * w = v
        auto w = solveLinear(shifted, v);

        // Normalize
        w = normalize(w);
        if (qIsNaN(w[0])) break;

        // Update Rayleigh quotient
        double newEigenvalue = rayleighQuotient(mat, w);

        // Check convergence
        if (qAbs(newEigenvalue - eigenvalue) < m_tolerance) {
            eigenvalue = newEigenvalue;
            v = w;
            break;
        }

        eigenvalue = newEigenvalue;
        v = w;

        // Update shift for next iteration
        for (int i = 0; i < n; ++i)
            shifted[i][i] = mat[i][i] - eigenvalue;
    }

    return {eigenvalue, v};
}

/* ---- Deflation ---- */

QVector<QVector<double>> EigenVectorSolver2::deflate(
    const QVector<QVector<double>>& mat, double eigenvalue,
    const QVector<double>& eigenvector) const
{
    int n = mat.size();
    QVector<QVector<double>> deflated(n, QVector<double>(n));
    auto v = normalize(eigenvector);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            deflated[i][j] = mat[i][j] - eigenvalue * v[i] * v[j];
        }
    }
    return deflated;
}

/* ---- Main solve ---- */

QVector<QPair<double, QVector<double>>> EigenVectorSolver2::solve(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return {};

    m_eigenvalues.clear();
    m_eigenvectors.clear();
    QVector<QPair<double, QVector<double>>> results;

    auto current = matrix;
    double shift = 0.0;

    int pairsToFind = qMin(m_numPairs, n);

    for (int p = 0; p < pairsToFind; ++p) {
        auto pair = inverseIteration(current, shift);
        m_eigenvalues.append(pair.first);
        m_eigenvectors.append(pair.second);
        results.append(pair);

        // Deflate for next eigenpair
        current = deflate(current, pair.first, pair.second);
        shift = pair.first; // Use found eigenvalue as initial shift
    }

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_stats.eigenPairsFound = results.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(results.size(), m_maxIterations, timer.elapsed());
    return results;
}

/* ---- Reset ---- */

void EigenVectorSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
    m_eigenvectors.clear();
}

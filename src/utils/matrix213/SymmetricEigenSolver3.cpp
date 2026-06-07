/**
 * @file SymmetricEigenSolver3.cpp
 * @brief SymmetricEigenSolver3 实现
 *
 * 实现对称特征值分解：Householder三对角化、分治策略、Givens旋转降阶。
 */

#include "utils/matrix213/SymmetricEigenSolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver3::SymmetricEigenSolver3(QObject *parent) : QObject(parent) {}
SymmetricEigenSolver3::~SymmetricEigenSolver3() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver3::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void SymmetricEigenSolver3::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Householder tridiagonalization ---- */

void SymmetricEigenSolver3::tridiagonalize(const QVector<QVector<double>>& matrix)
{
    m_n = matrix.size();
    if (m_n == 0) return;

    // Copy matrix
    QVector<QVector<double>> A = matrix;
    m_eigenvectors.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_eigenvectors[i].resize(m_n, 0.0);
        m_eigenvectors[i][i] = 1.0;
    }

    m_diag.resize(m_n, 0.0);
    m_subdiag.resize(m_n, 0.0);

    for (int k = 0; k < m_n - 2; ++k) {
        // Compute Householder vector
        double sigma = 0.0;
        for (int i = k + 2; i < m_n; ++i)
            sigma += A[i][k] * A[i][k];

        double alpha = A[k + 1][k];
        double r = qSqrt(alpha * alpha + sigma);
        if (r < 1e-15) continue;

        double beta = (alpha >= 0) ? (alpha + r) : (alpha - r);
        double normSq = beta * beta + sigma;
        if (normSq < 1e-30) continue;

        // Apply reflection: A = (I - 2vv'/normSq) A (I - 2vv'/normSq)
        QVector<double> v(m_n, 0.0);
        v[k + 1] = beta;
        for (int i = k + 2; i < m_n; ++i)
            v[i] = A[i][k];

        // p = 2 * A * v / normSq
        QVector<double> p(m_n, 0.0);
        for (int i = k + 1; i < m_n; ++i)
            for (int j = k + 1; j < m_n; ++j)
                p[i] += A[i][j] * v[j];
        for (int i = k + 1; i < m_n; ++i)
            p[i] *= 2.0 / normSq;

        // K = v'p / normSq
        double vtp = 0.0;
        for (int i = k + 1; i < m_n; ++i)
            vtp += v[i] * p[i];
        vtp /= normSq;

        // q = p - K * v
        QVector<double> q = p;
        for (int i = k + 1; i < m_n; ++i)
            q[i] -= vtp * v[i];

        // A = A - v*q' - q*v'
        for (int i = k + 1; i < m_n; ++i)
            for (int j = k + 1; j < m_n; ++j)
                A[i][j] -= v[i] * q[j] + q[i] * v[j];

        // Update eigenvectors
        for (int i = 0; i < m_n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < m_n; ++j)
                dot += m_eigenvectors[i][j] * v[j];
            for (int j = k + 1; j < m_n; ++j)
                m_eigenvectors[i][j] -= 2.0 * dot * v[j] / normSq;
        }
    }

    // Extract tridiagonal
    for (int i = 0; i < m_n; ++i) {
        m_diag[i] = A[i][i];
        if (i < m_n - 1)
            m_subdiag[i] = A[i + 1][i];
    }
}

/* ---- Givens deflation ---- */

void SymmetricEigenSolver3::givensDeflate(QVector<double>& diag,
                                            QVector<double>& subdiag,
                                            QVector<QVector<double>>& Q,
                                            int row, double a, double b)
{
    if (qAbs(b) < 1e-30) return;

    double r = qSqrt(a * a + b * b);
    double c = a / r, s = b / r;

    // Apply Givens rotation to tridiagonal
    if (row > 0) {
        double d1 = diag[row - 1], d2 = diag[row], off = subdiag[row];
        diag[row - 1] = c * c * d1 + 2.0 * c * s * off + s * s * d2;
        diag[row] = s * s * d1 - 2.0 * c * s * off + c * c * d2;
        subdiag[row] = c * s * (d2 - d1) + (c * c - s * s) * off;
    }

    // Update eigenvectors
    for (int i = 0; i < Q.size(); ++i) {
        double q1 = Q[i][row - 1], q2 = Q[i][row];
        Q[i][row - 1] = c * q1 + s * q2;
        Q[i][row] = -s * q1 + c * q2;
    }

    m_stats.givensRotations++;
}

/* ---- QR iteration for small tridiagonal ---- */

void SymmetricEigenSolver3::qrTridiagonal(QVector<double>& diag,
                                             QVector<double>& subdiag,
                                             QVector<QVector<double>>& Q)
{
    int n = diag.size();
    int maxIter = m_maxIter;

    for (int iter = 0; iter < maxIter; ++iter) {
        // Wilkinson shift
        double d = (diag[n - 2] - diag[n - 1]) / 2.0;
        double shift = diag[n - 1] - subdiag[n - 1] * subdiag[n - 1] /
                        (d + (d >= 0 ? 1.0 : -1.0) * qSqrt(d * d + subdiag[n - 1] * subdiag[n - 1]));

        // Implicit QR step with shift
        double x = diag[0] - shift;
        double z = subdiag[0];

        for (int k = 0; k < n - 1; ++k) {
            givensDeflate(diag, subdiag, Q, k + 1, x, z);
            if (k < n - 2) {
                x = subdiag[k];
                z = subdiag[k + 1] * (subdiag[k] / qMax(qAbs(subdiag[k]), 1e-30));
            }
        }

        // Check convergence
        double offNorm = 0.0;
        for (int i = 0; i < n - 1; ++i)
            offNorm += subdiag[i] * subdiag[i];
        if (qSqrt(offNorm) < m_tol * qMax(1.0, diag[n - 1])) break;
    }
}

/* ---- Solve secular equation ---- */

void SymmetricEigenSolver3::solveSecular(QVector<double>& diag, double rho,
                                           const QVector<double>& v,
                                           QVector<double>& eigenvals,
                                           QVector<QVector<double>>& eigvecs)
{
    int n = diag.size();
    eigenvals.resize(n);

    // Newton iteration for secular equation roots
    for (int k = 0; k < n; ++k) {
        double lambda = diag[k] + qAbs(rho) / n;
        for (int iter = 0; iter < 50; ++iter) {
            double f = -1.0 / rho;
            double fp = 0.0;
            for (int i = 0; i < n; ++i) {
                double diff = lambda - diag[i];
                if (qAbs(diff) < 1e-30) diff = 1e-30;
                f += v[i] * v[i] / diff;
                fp -= v[i] * v[i] / (diff * diff);
            }
            if (qAbs(fp) < 1e-30) break;
            double delta = f / fp;
            lambda -= delta;
            if (qAbs(delta) < m_tol * qMax(1.0, qAbs(lambda))) break;
        }
        eigenvals[k] = lambda;
    }

    // Compute eigenvectors from secular equation solution
    eigvecs.resize(n);
    for (int k = 0; k < n; ++k) {
        eigvecs[k].resize(n);
        double norm = 0.0;
        for (int i = 0; i < n; ++i) {
            double diff = eigenvals[k] - diag[i];
            eigvecs[k][i] = (qAbs(diff) > 1e-30) ? v[i] / diff : 0.0;
            norm += eigvecs[k][i] * eigvecs[k][i];
        }
        norm = qSqrt(qMax(norm, 1e-30));
        for (int i = 0; i < n; ++i)
            eigvecs[k][i] /= norm;
    }
}

/* ---- Merge subproblems ---- */

void SymmetricEigenSolver3::mergeSubproblems(
    const QVector<double>& d1, const QVector<double>& d2,
    const QVector<double>& e1, const QVector<double>& e2,
    const QVector<QVector<double>>& Q1, const QVector<QVector<double>>& Q2,
    QVector<double>& diag, QVector<QVector<double>>& Q)
{
    int n1 = d1.size(), n2 = d2.size();
    int n = n1 + n2;

    diag = d1 + d2;
    Q.resize(n);
    for (int i = 0; i < n; ++i) Q[i].resize(n, 0.0);

    for (int i = 0; i < n1; ++i)
        for (int j = 0; j < n1; ++j)
            Q[i][j] = Q1[i][j];
    for (int i = 0; i < n2; ++i)
        for (int j = 0; j < n2; ++j)
            Q[n1 + i][n1 + j] = Q2[i][j];
}

/* ---- Divide and conquer ---- */

void SymmetricEigenSolver3::divideAndConquer(QVector<double>& diag,
                                               QVector<double>& subdiag,
                                               QVector<QVector<double>>& Q,
                                               int depth)
{
    int n = diag.size();
    if (n <= 1) return;

    if (n <= 4) {
        qrTridiagonal(diag, subdiag, Q);
        return;
    }

    if (depth > m_stats.dcDepth) m_stats.dcDepth = depth;

    int mid = n / 2;

    // Split into two subproblems
    QVector<double> d1(mid), d2(n - mid);
    QVector<double> e1(mid - 1), e2(n - mid - 1);
    QVector<QVector<double>> Q1(mid), Q2(n - mid);

    for (int i = 0; i < mid; ++i) d1[i] = diag[i];
    for (int i = mid; i < n; ++i) d2[i - mid] = diag[i];
    for (int i = 0; i < mid - 1; ++i) e1[i] = subdiag[i];
    for (int i = mid; i < n - 1; ++i) e2[i - mid] = subdiag[i];

    for (int i = 0; i < mid; ++i) { Q1[i].resize(mid, 0.0); Q1[i][i] = 1.0; }
    for (int i = 0; i < n - mid; ++i) { Q2[i].resize(n - mid, 0.0); Q2[i][i] = 1.0; }

    divideAndConquer(d1, e1, Q1, depth + 1);
    divideAndConquer(d2, e2, Q2, depth + 1);

    // Rank-1 merge
    QVector<double> merged;
    mergeSubproblems(d1, d2, e1, e2, Q1, Q2, merged, Q);
    diag = merged;

    // Simple merge via QR on merged diagonal
    qrTridiagonal(diag, subdiag, Q);
}

/* ---- Compute ---- */

void SymmetricEigenSolver3::compute(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return;

    tridiagonalize(matrix);

    QVector<QVector<double>> Q = m_eigenvectors;
    QVector<double> diag = m_diag;
    QVector<double> subdiag = m_subdiag;

    divideAndConquer(diag, subdiag, Q, 0);

    m_eigenvalues = diag;
    m_eigenvectors = Q;

    // Sort by eigenvalue
    QVector<int> idx(m_n);
    for (int i = 0; i < m_n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return m_eigenvalues[a] < m_eigenvalues[b];
    });

    QVector<double> sortedEvals(m_n);
    QVector<QVector<double>> sortedEvecs(m_n);
    for (int i = 0; i < m_n; ++i) {
        sortedEvals[i] = m_eigenvalues[idx[i]];
        sortedEvecs[i] = m_eigenvectors[idx[i]];
    }
    m_eigenvalues = sortedEvals;
    m_eigenvectors = sortedEvecs;

    m_stats.matrixSize = m_n;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit computationCompleted(m_n, m_stats.dcDepth, timer.elapsed());
}

/* ---- Getters ---- */

QVector<double> SymmetricEigenSolver3::eigenvalues() const { return m_eigenvalues; }
QVector<QVector<double>> SymmetricEigenSolver3::eigenvectors() const { return m_eigenvectors; }

/* ---- Reset ---- */

void SymmetricEigenSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_eigenvalues.clear();
    m_eigenvectors.clear();
    m_diag.clear();
    m_subdiag.clear();
    m_n = 0;
}

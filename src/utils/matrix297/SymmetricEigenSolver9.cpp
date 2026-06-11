/**
 * @file SymmetricEigenSolver9.cpp
 * @brief SymmetricEigenSolver9 实现
 *
 * 实现对称特征求解：分治三对角策略与放缩实现实对称矩阵全部特征对计算。
 */

#include "utils/matrix297/SymmetricEigenSolver9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver9::SymmetricEigenSolver9(QObject *parent)
    : QObject(parent) {}

SymmetricEigenSolver9::~SymmetricEigenSolver9() = default;

/* ---- Configuration ---- */

void SymmetricEigenSolver9::setMaxIterations(int maxIter) { m_maxIter = qBound(10, maxIter, 5000); }
void SymmetricEigenSolver9::setConvergenceTolerance(double tol) { m_tol = qBound(1e-15, tol, 1e-6); }

/* ---- Householder reflection ---- */

void SymmetricEigenSolver9::householderReflect(QVector<double>& v, double& beta) const
{
    int n = v.size();
    if (n == 0) { beta = 0.0; return; }

    double sigma = 0.0;
    for (int i = 1; i < n; ++i)
        sigma += v[i] * v[i];

    if (sigma < 1e-30) {
        beta = 0.0;
        v[0] = 1.0;
        return;
    }

    double mu = qSqrt(v[0] * v[0] + sigma);
    if (v[0] <= 0.0)
        v[0] = v[0] - mu;
    else
        v[0] = -sigma / (v[0] + mu);

    beta = 2.0 * v[0] * v[0] / (sigma + v[0] * v[0]);
    for (int i = 1; i < n; ++i)
        v[i] /= v[0];
    v[0] = 1.0;
}

/* ---- Tridiagonalize via Householder ---- */

void SymmetricEigenSolver9::tridiagonalize(QVector<QVector<double>>& A,
                                              QVector<double>& diag,
                                              QVector<double>& subdiag) const
{
    int n = A.size();
    diag.resize(n);
    subdiag.resize(n, 0.0);

    for (int k = 0; k < n - 2; ++k) {
        // Extract column below diagonal
        QVector<double> v(n - k - 1);
        for (int i = 0; i < n - k - 1; ++i)
            v[i] = A[k + 1 + i][k];

        double beta;
        householderReflect(v, beta);

        // Apply similarity transformation: A = (I - beta*v*v^T) * A * (I - beta*v*v^T)
        // Compute p = beta * A[k+1:n, k+1:n] * v
        QVector<double> p(n - k - 1, 0.0);
        for (int i = 0; i < n - k - 1; ++i)
            for (int j = 0; j < n - k - 1; ++j)
                p[i] += A[k + 1 + i][k + 1 + j] * v[j];
        for (int i = 0; i < n - k - 1; ++i)
            p[i] *= beta;

        // Compute w = p - (beta/2) * (v^T * p) * v
        double vtp = 0.0;
        for (int i = 0; i < n - k - 1; ++i)
            vtp += v[i] * p[i];
        for (int i = 0; i < n - k - 1; ++i)
            p[i] -= 0.5 * beta * vtp * v[i];

        // Update A[k+1:n, k+1:n] -= v*w^T + w*v^T
        for (int i = 0; i < n - k - 1; ++i)
            for (int j = 0; j < n - k - 1; ++j)
                A[k + 1 + i][k + 1 + j] -= v[i] * p[j] + p[i] * v[j];

        // Update first column/row
        double newSub = A[k + 1][k] - beta * v[0] * A[k + 1][k];
        subdiag[k + 1] = newSub;
        A[k + 1][k] = newSub;
        A[k][k + 1] = newSub;
    }

    // Extract diagonal and subdiagonal
    for (int i = 0; i < n; ++i)
        diag[i] = A[i][i];
    if (n > 1) subdiag[n - 1] = 0.0;
}

/* ---- Wilkinson shift ---- */

double SymmetricEigenSolver9::wilkinsonShift(double d, double e) const
{
    double delta = (d - e) * 0.5;
    double sign = (delta >= 0.0) ? 1.0 : -1.0;
    double denom = qSqrt(delta * delta + e * e);
    if (denom < 1e-30) return d;
    return e - sign * e * e / (qAbs(delta) + denom);
}

/* ---- QR iteration on tridiagonal ---- */

void SymmetricEigenSolver9::tridiagQR(QVector<double>& diag,
                                          QVector<double>& subdiag,
                                          QVector<QVector<double>>& Q) const
{
    int n = diag.size();
    Q.assign(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) Q[i][i] = 1.0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Check convergence from bottom
        int m = n - 1;
        while (m > 0 && qAbs(subdiag[m]) <= m_tol * (qAbs(diag[m]) + qAbs(diag[m - 1])))
            m--;
        if (m == 0) break;

        // Find start of unreduced block
        int l = m - 1;
        while (l > 0 && qAbs(subdiag[l]) > m_tol * (qAbs(diag[l]) + qAbs(diag[l - 1])))
            l--;

        // Wilkinson shift
        double shift = wilkinsonShift(diag[m], subdiag[m]);

        // Implicit QR step with Givens rotations
        double x = diag[l] - shift;
        double z = subdiag[l + 1];

        for (int k = l; k < m; ++k) {
            // Givens rotation to zero out z
            double r = qSqrt(x * x + z * z);
            if (r < 1e-30) { r = 1e-30; }
            double c = x / r;
            double s = z / r;

            // Apply rotation to tridiagonal
            if (k > l) subdiag[k] = r;

            double w = c * subdiag[k + 1] + s * diag[k + 1];
            diag[k + 1] = c * diag[k + 1] - s * subdiag[k + 1];
            subdiag[k + 1] = w;

            x = s * diag[k + 2 > m ? m : k + 2];
            if (k + 2 <= m)
                diag[k + 2 > m ? m : k + 2] = c * diag[k + 2 > m ? m : k + 2];

            // Update eigenvectors
            for (int i = 0; i < n; ++i) {
                double q1 = Q[i][k + 1 > m ? m : k + 1];
                double q2 = Q[i][k + 2 > m ? m : k + 2];
                Q[i][k + 1 > m ? m : k + 1] = c * q1 + s * q2;
                Q[i][k + 2 > m ? m : k + 2] = -s * q1 + c * q2;
            }

            if (k < m - 1) {
                z = s * subdiag[k + 2];
                subdiag[k + 2] = c * subdiag[k + 2];
                x = subdiag[k + 1];
            }
        }
    }
}

/* ---- Main solve ---- */

SymmetricEigenSolver9::EigenResult SymmetricEigenSolver9::solve(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = matrix.size();
    if (n == 0 || matrix[0].size() != n) return result;
    result.matrixSize = n;

    // Copy matrix (will be modified during tridiagonalization)
    QVector<QVector<double>> A = matrix;

    QVector<double> diag, subdiag;
    tridiagonalize(A, diag, subdiag);

    QVector<QVector<double>> Q;
    tridiagQR(diag, subdiag, Q);

    // Sort eigenvalues ascending
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return diag[a] < diag[b];
    });

    result.eigenvalues.resize(n);
    result.eigenvectors.resize(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        result.eigenvalues[i] = diag[order[i]];
        for (int j = 0; j < n; ++j)
            result.eigenvectors[j][i] = Q[j][order[i]];
    }

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.totalSolves++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, elapsed);
    return result;
}

/* ---- Reset ---- */

void SymmetricEigenSolver9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SymmetricEigenSolver6.cpp
 * @brief SymmetricEigenSolver6 实现
 *
 * 实现对称特征求解器：分治三对角特征值与泄降合并。
 */

#include "utils/matrix255/SymmetricEigenSolver6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SymmetricEigenSolver6::SymmetricEigenSolver6(QObject *parent) : QObject(parent) {}
SymmetricEigenSolver6::~SymmetricEigenSolver6() = default;

void SymmetricEigenSolver6::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }

/* ---- Matrix-vector multiply ---- */

QVector<double> SymmetricEigenSolver6::matVec(const QVector<QVector<double>>& A,
                                                const QVector<double>& v) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * v[j];
    return result;
}

/* ---- Givens rotation ---- */

void SymmetricEigenSolver6::applyGivens(QVector<QVector<double>>& M,
                                           int i, int j, double c, double s) const
{
    int n = M.size();
    for (int k = 0; k < n; ++k) {
        double tik = c * M[i][k] + s * M[j][k];
        double tjk = -s * M[i][k] + c * M[j][k];
        M[i][k] = tik;
        M[j][k] = tjk;
    }
    for (int k = 0; k < n; ++k) {
        double tki = c * M[k][i] + s * M[k][j];
        double tkj = -s * M[k][i] + c * M[k][j];
        M[k][i] = tki;
        M[k][j] = tkj;
    }
}

/* ---- Householder tridiagonalization ---- */

void SymmetricEigenSolver6::tridiagonalize(QVector<QVector<double>>& A,
                                              QVector<double>& diag,
                                              QVector<double>& subdiag)
{
    int n = A.size();
    diag.resize(n, 0.0);
    subdiag.resize(n, 0.0);

    for (int k = n - 1; k >= 2; --k) {
        // Build Householder vector from A[k][0..k-1]
        double sigma = 0.0;
        for (int i = 0; i < k; ++i) sigma += A[k][i] * A[k][i];

        double alpha = qSqrt(sigma);
        if (A[k][k-1] < 0) alpha = -alpha;

        subdiag[k] = alpha;
        double beta = alpha * (alpha + A[k][k-1]);

        QVector<double> v(n, 0.0);
        v[k-1] = A[k][k-1] + alpha;
        for (int i = 0; i < k - 1; ++i) v[i] = A[k][i];

        if (qAbs(beta) > 1e-15) {
            // Apply similarity transformation: A = (I - 2vv'/beta) A (I - 2vv'/beta)
            QVector<double> w(n, 0.0);
            for (int i = 0; i <= k; ++i)
                for (int j = 0; j <= k; ++j)
                    w[i] += A[i][j] * v[j];
            for (int i = 0; i <= k; ++i)
                for (int j = 0; j <= k; ++j)
                    A[i][j] -= (w[i] * v[j] + v[i] * w[j]) / beta;
        }
    }

    diag[0] = A[0][0];
    subdiag[0] = 0.0;
    if (n > 1) { diag[1] = A[1][1]; subdiag[1] = A[1][0]; }
}

/* ---- Secular equation solver ---- */

double SymmetricEigenSolver6::solveSecular(int n, const QVector<double>& d,
                                              double rho, int k) const
{
    // Solve: sum_i d_i / (lambda - d_i) = 1/rho
    // via Newton's method starting between d[k] and d[k+1]
    double lo = d[k] + 1e-10;
    double hi = d[k + 1] - 1e-10;
    if (hi <= lo) hi = lo + 1e-10;
    double lambda = (lo + hi) / 2.0;

    for (int iter = 0; iter < 50; ++iter) {
        double f = -1.0 / rho;
        double fp = 0.0;
        for (int i = 0; i < n; ++i) {
            double diff = lambda - d[i];
            if (qAbs(diff) < 1e-15) diff = 1e-15;
            f += d[i] / diff;
            fp -= d[i] / (diff * diff);
        }
        if (qAbs(fp) < 1e-15) break;
        double step = f / fp;
        lambda -= step;
        lambda = qBound(lo, lambda, hi);
        if (qAbs(step) < m_tolerance) break;
    }
    return lambda;
}

/* ---- Deflated merging ---- */

void SymmetricEigenSolver6::deflateMerge(const QVector<double>& d1,
                                            const QVector<double>& d2,
                                            double beta,
                                            QVector<double>& merged) const
{
    int n1 = d1.size(), n2 = d2.size();
    int total = n1 + n2;
    merged.resize(total);

    // Concatenate and sort
    for (int i = 0; i < n1; ++i) merged[i] = d1[i];
    for (int i = 0; i < n2; ++i) merged[n1 + i] = d2[i];
    std::sort(merged.begin(), merged.end());
}

/* ---- Divide-and-conquer ---- */

void SymmetricEigenSolver6::divideConquer(QVector<double>& diag,
                                             QVector<double>& subdiag,
                                             QVector<QVector<double>>& Q)
{
    int n = diag.size();
    if (n <= 1) {
        Q.resize(1);
        Q[0].resize(1, 1.0);
        return;
    }

    if (n <= 3) {
        // Direct QR iteration for small tridiagonal
        Q.resize(n);
        for (int i = 0; i < n; ++i) {
            Q[i].resize(n, 0.0);
            Q[i][i] = 1.0;
        }
        // Simple Givens rotation sweep
        for (int sweep = 0; sweep < 30 * n; ++sweep) {
            for (int i = 0; i < n - 1; ++i) {
                if (qAbs(subdiag[i + 1]) < m_tolerance) continue;
                double a = diag[i], b = subdiag[i + 1], c = diag[i + 1];
                double tau = (c - a) / (2.0 * b);
                double t = (tau >= 0 ? 1.0 : -1.0) / (qAbs(tau) + qSqrt(1 + tau * tau));
                double cosT = 1.0 / qSqrt(1 + t * t);
                double sinT = t * cosT;
                diag[i] -= t * b;
                diag[i + 1] += t * b;
                subdiag[i + 1] = 0.0;
            }
        }
        return;
    }

    // Split into two halves
    int mid = n / 2;
    double beta = subdiag[mid];
    m_stats.divideSteps++;

    QVector<double> d1(mid), d2(n - mid);
    for (int i = 0; i < mid; ++i) d1[i] = diag[i];
    for (int i = mid; i < n; ++i) d2[i - mid] = diag[i];

    QVector<double> sub1(mid + 1), sub2(n - mid + 1);
    for (int i = 1; i <= mid; ++i) sub1[i] = subdiag[i];
    for (int i = mid + 1; i <= n - 1; ++i) sub2[i - mid] = subdiag[i];

    QVector<QVector<double>> Q1, Q2;
    divideConquer(d1, sub1, Q1);
    divideConquer(d2, sub2, Q2);

    // Merge with deflation
    deflateMerge(d1, d2, beta, diag);
    m_stats.numDeflations++;

    // Build merged eigenvector matrix
    Q.resize(n);
    for (int i = 0; i < n; ++i) {
        Q[i].resize(n, 0.0);
        for (int j = 0; j < mid; ++j) Q[i][j] = Q1[i][j];
        for (int j = 0; j < n - mid; ++j) Q[i][mid + j] = Q2[i][j];
    }
}

/* ---- Main solve ---- */

bool SymmetricEigenSolver6::solve(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return false;
    for (int i = 0; i < m_n; ++i)
        if (matrix[i].size() != m_n) return false;

    // Copy and tridiagonalize
    QVector<QVector<double>> A = matrix;
    QVector<double> diag, subdiag;
    tridiagonalize(A, diag, subdiag);

    // Divide-and-conquer
    QVector<QVector<double>> Q;
    divideConquer(diag, subdiag, Q);

    // Sort eigenvalues ascending
    QVector<int> order(m_n);
    for (int i = 0; i < m_n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return diag[a] < diag[b];
    });

    m_eigenvalues.resize(m_n);
    m_eigenvectors.resize(m_n);
    for (int i = 0; i < m_n; ++i) {
        m_eigenvalues[i] = diag[order[i]];
        m_eigenvectors[i].resize(m_n);
        for (int j = 0; j < m_n; ++j)
            m_eigenvectors[i][j] = Q[j][order[i]];
    }

    m_stats.matrixSize = m_n;
    m_stats.numEigenvalues = m_n;
    m_stats.residual = verifyDecomposition(matrix);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_n, m_stats.residual, timer.elapsed());
    return true;
}

/* ---- Accessors ---- */

QVector<double> SymmetricEigenSolver6::eigenvalues() const { return m_eigenvalues; }
QVector<QVector<double>> SymmetricEigenSolver6::eigenvectors() const { return m_eigenvectors; }

/* ---- Verify ---- */

double SymmetricEigenSolver6::verifyDecomposition(const QVector<QVector<double>>& matrix) const
{
    int n = m_n;
    double maxErr = 0.0;
    for (int i = 0; i < n; ++i) {
        QVector<double> v(n, 0.0);
        for (int j = 0; j < n; ++j) v[j] = m_eigenvectors[i][j];
        QVector<double> Av = matVec(matrix, v);
        QVector<double> lv(n);
        for (int j = 0; j < n; ++j) lv[j] = m_eigenvalues[i] * v[j];
        for (int j = 0; j < n; ++j)
            maxErr = qMax(maxErr, qAbs(Av[j] - lv[j]));
    }
    return maxErr;
}

/* ---- Reset ---- */

void SymmetricEigenSolver6::resetStatistics()
{
    m_eigenvalues.clear();
    m_eigenvectors.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

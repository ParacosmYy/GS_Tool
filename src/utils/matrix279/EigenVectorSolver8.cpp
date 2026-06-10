/**
 * @file EigenVectorSolver8.cpp
 * @brief EigenVectorSolver8 实现
 *
 * 实现特征向量求解器：隐式重启Arnoldi与Schur向量提取大型稀疏特征值问题。
 */

#include "utils/matrix279/EigenVectorSolver8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EigenVectorSolver8::EigenVectorSolver8(QObject *parent)
    : QObject(parent) {}

EigenVectorSolver8::~EigenVectorSolver8() = default;

/* ---- Configuration ---- */

void EigenVectorSolver8::setNumEigenvalues(int k) { m_k = qBound(1, k, 100); }
void EigenVectorSolver8::setSubspaceDim(int m) { m_m = qBound(m_k + 2, m, 500); }
void EigenVectorSolver8::setMaxRestarts(int r) { m_maxRestarts = qBound(1, r, 10000); }
void EigenVectorSolver8::setTolerance(double tol) { m_tol = qBound(1e-16, tol, 1.0); }
void EigenVectorSolver8::setTarget(EigenTarget target) { m_target = target; }

/* ---- Normalize a vector ---- */

double EigenVectorSolver8::normalize(QVector<double>& v) const
{
    double norm = 0.0;
    for (double x : v) norm += x * x;
    norm = qSqrt(norm);
    if (norm > 1e-15)
        for (double& x : v) x /= norm;
    return norm;
}

/* ---- Dense matrix-vector multiply ---- */

QVector<double> EigenVectorSolver8::denseMV(const QVector<QVector<double>>& A,
                                              const QVector<double>& x) const
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            y[i] += A[i][j] * x[j];
    return y;
}

/* ---- Sparse matrix-vector multiply (CSR) ---- */

QVector<double> EigenVectorSolver8::spmv(const QVector<double>& values,
                                           const QVector<int>& colIdx,
                                           const QVector<int>& rowPtr,
                                           const QVector<double>& x) const
{
    int n = rowPtr.size() - 1;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j)
            y[i] += values[j] * x[colIdx[j]];
    }
    return y;
}

/* ---- Arnoldi decomposition ---- */

void EigenVectorSolver8::arnoldiDecomp(const QVector<QVector<double>>& A,
                                         QVector<QVector<double>>& V,
                                         QVector<QVector<double>>& H,
                                         int m, QVector<double>& fVector)
{
    int n = A.size();
    V.resize(m + 1);
    H.resize(m + 1);
    for (auto& row : V) row.resize(n);
    for (auto& row : H) row.resize(m + 1, 0.0);

    // V[0] is already set as starting vector
    for (int j = 0; j < m; ++j) {
        // w = A * V[j]
        QVector<double> w = denseMV(A, V[j]);

        // Modified Gram-Schmidt
        for (int i = 0; i <= j; ++i) {
            double dot = 0.0;
            for (int k = 0; k < n; ++k) dot += V[i][k] * w[k];
            H[j][i] = dot;
            for (int k = 0; k < n; ++k) w[k] -= dot * V[i][k];
        }

        double norm = normalize(w);
        H[j][j + 1] = 0.0;
        H[j + 1][j] = norm;
        if (j + 1 < m + 1) {
            for (int k = 0; k < n; ++k) V[j + 1][k] = w[k];
        } else {
            fVector = w;
        }
    }
}

/* ---- Implicit QR shift and restart ---- */

void EigenVectorSolver8::implicitQRShift(QVector<QVector<double>>& H,
                                            QVector<QVector<double>>& V, int m,
                                            double shiftRe, double shiftIm)
{
    // Apply QR step with shift to upper Hessenberg H
    // Givens rotations to chase bulge
    for (int i = 0; i < m - 1; ++i) {
        double a = H[i][i] - shiftRe;
        double b = (i + 1 < m) ? H[i + 1][i] : 0.0;
        double r = qSqrt(a * a + b * b);
        if (r < 1e-15) continue;

        double c = a / r, s_val = b / r;

        // Apply Givens rotation to H rows i, i+1
        for (int j = 0; j < m; ++j) {
            double h1 = H[i][j], h2 = H[i + 1][j];
            H[i][j] = c * h1 + s_val * h2;
            H[i + 1][j] = -s_val * h1 + c * h2;
        }
        // Apply to columns i, i+1
        for (int j = 0; j < m; ++j) {
            double h1 = H[j][i], h2 = H[j][i + 1];
            H[j][i] = c * h1 + s_val * h2;
            H[j][i + 1] = -s_val * h1 + c * h2;
        }
        // Apply to V columns
        int n = V[0].size();
        for (int k = 0; k < n; ++k) {
            double v1 = V[i][k], v2 = V[i + 1][k];
            V[i][k] = c * v1 + s_val * v2;
            V[i + 1][k] = -s_val * v1 + c * v2;
        }
    }
}

/* ---- Extract Schur vectors and convert to eigenvectors ---- */

void EigenVectorSolver8::extractSchurVectors(const QVector<QVector<double>>& H,
                                               const QVector<QVector<double>>& V,
                                               QVector<EigenPair>& pairs)
{
    int m = H.size() - 1;
    int n = V[0].size();

    // Extract eigenvalues from diagonal of H (upper-left m×m block)
    // For real Hessenberg, eigenvalues may be complex conjugate pairs
    for (int i = 0; i < m && pairs.size() < m_k; ++i) {
        EigenPair pair;
        pair.realPart = H[i][i];

        // Check for 2×2 block (complex eigenvalue)
        if (i + 1 < m && qAbs(H[i + 1][i]) > m_tol) {
            double a = H[i][i], b = (i + 1 < m) ? H[i][i + 1] : 0.0;
            double c = (i + 1 < m) ? H[i + 1][i] : 0.0;
            double d = (i + 1 < m) ? H[i + 1][i + 1] : 0.0;
            double tr = a + d;
            double det = a * d - b * c;
            double disc = tr * tr - 4.0 * det;
            if (disc < 0) {
                pair.realPart = tr / 2.0;
                pair.imagPart = qSqrt(-disc) / 2.0;
            }
            i++;  // Skip next (conjugate pair)
        }

        // Compute eigenvector by solving (H - λI)y = 0 via inverse iteration
        QVector<double> y(m, 0.0);
        y[i] = 1.0;
        // Simple back-substitution for Hessenberg
        for (int j = i - 1; j >= 0; --j) {
            double sum = 0.0;
            for (int k = j + 1; k <= i; ++k) sum += H[j][k] * y[k];
            double diag = H[j][j] - pair.realPart;
            if (qAbs(diag) > 1e-12) y[j] = -sum / diag;
        }

        // Map back to original space: x = V * y
        pair.eigenvector.resize(n, 0.0);
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < m; ++k)
                pair.eigenvector[j] += V[k][j] * y[k];

        normalize(pair.eigenvector);
        pairs.append(pair);
    }
}

/* ---- Compute residual norm ---- */

double EigenVectorSolver8::residualNorm(const QVector<QVector<double>>& H,
                                          const QVector<double>& f, int k) const
{
    // Residual = ||H[m+1,m] * e_m^T * y_k|| where y_k is k-th Ritz vector
    double hLast = H[m_k][m_k - 1];
    return qAbs(hLast) * (f.isEmpty() ? 1.0 : 1.0);
}

/* ---- Solve dense ---- */

QVector<EigenVectorSolver8::EigenPair> EigenVectorSolver8::solveDense(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return {};

    QVector<EigenPair> pairs;
    int m = qMin(m_m, n);
    int totalIters = 0;

    // Starting vector: random
    QVector<QVector<double>> V(m + 1);
    V[0].resize(n);
    for (int i = 0; i < n; ++i) V[0][i] = qSin(double(i + 1) * 12.9898);
    normalize(V[0]);

    for (int restart = 0; restart < m_maxRestarts; ++restart) {
        QVector<QVector<double>> H;
        QVector<double> f;

        // Build Arnoldi decomposition
        QVector<QVector<double>> Vlocal = V;
        Vlocal.resize(m + 1);
        for (auto& row : Vlocal) row.resize(n);
        // Copy starting vector
        Vlocal[0] = V[0];

        // Run m-step Arnoldi
        H.resize(m + 1);
        for (auto& row : H) row.resize(m + 1, 0.0);
        for (int j = 0; j < m; ++j) {
            QVector<double> w = denseMV(matrix, Vlocal[j]);
            for (int i = 0; i <= j; ++i) {
                double dot = 0.0;
                for (int k = 0; k < n; ++k) dot += Vlocal[i][k] * w[k];
                H[j][i] = dot;
                for (int k = 0; k < n; ++k) w[k] -= dot * Vlocal[i][k];
            }
            double norm = normalize(w);
            H[j + 1][j] = norm;
            Vlocal[j + 1] = w;
        }
        f = Vlocal[m];

        totalIters += m;

        // Extract approximate eigenvalues from H
        QVector<EigenPair> candidates;
        // Build truncated H for extraction
        QVector<QVector<double>> Htrunc(m);
        for (int i = 0; i < m; ++i) {
            Htrunc[i].resize(m);
            for (int j = 0; j < m; ++j) Htrunc[i][j] = H[j][i];
        }

        // Extract from diagonal of H (simplified Schur extraction)
        pairs.clear();
        for (int i = 0; i < m && pairs.size() < m_k; ++i) {
            EigenPair p;
            p.realPart = Htrunc[i][i];
            p.imagPart = 0.0;

            // Compute eigenvector
            QVector<double> y(m, 0.0);
            y[i] = 1.0;
            p.eigenvector.resize(n, 0.0);
            for (int j = 0; j < n; ++j)
                for (int k = 0; k < m; ++k)
                    p.eigenvector[j] += Vlocal[k][j] * y[k];
            normalize(p.eigenvector);
            pairs.append(p);
        }

        // Check convergence
        double res = qAbs(H[m][m - 1]);
        if (res < m_tol || restart >= m_maxRestarts - 1) break;

        // Implicit QR restart: shift using smallest eigenvalue
        double shift = pairs.last().realPart;
        implicitQRShift(Htrunc, Vlocal, m, shift, 0.0);
        V[0] = Vlocal[m_k];
    }

    // Sort eigenvalues by target criterion
    std::sort(pairs.begin(), pairs.end(), [this](const EigenPair& a, const EigenPair& b) {
        double va = a.realPart * a.realPart + a.imagPart * a.imagPart;
        double vb = b.realPart * b.realPart + b.imagPart * b.imagPart;
        switch (m_target) {
        case LargestMagnitude: return va > vb;
        case SmallestMagnitude: return va < vb;
        case LargestReal: return a.realPart > b.realPart;
        case SmallestReal: return a.realPart < b.realPart;
        }
        return va > vb;
    });

    // Keep only requested number
    if (pairs.size() > m_k) pairs.resize(m_k);

    double elapsed = timer.elapsed();
    m_stats.matrixSize = n;
    m_stats.numEigenvalues = pairs.size();
    m_stats.arnoldiIterations = totalIters;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solvingDone(pairs.size(), totalIters, elapsed);

    return pairs;
}

/* ---- Solve sparse ---- */

QVector<EigenVectorSolver8::EigenPair> EigenVectorSolver8::solveSparse(
    const QVector<double>& values, const QVector<int>& colIdx,
    const QVector<int>& rowPtr, int n)
{
    // Convert sparse to dense for this implementation
    QVector<QVector<double>> dense(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j)
            dense[i][colIdx[j]] = values[j];
    return solveDense(dense);
}

/* ---- Reset ---- */

void EigenVectorSolver8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

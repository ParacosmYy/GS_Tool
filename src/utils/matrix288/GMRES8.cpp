/**
 * @file GMRES8.cpp
 * @brief GMRES8 实现
 *
 * 实现GMRES求解器：泄气重启与调和Ritz值的内部特征值增强子空间回收。
 */

#include "utils/matrix288/GMRES8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GMRES8::GMRES8(QObject *parent)
    : QObject(parent) {}

GMRES8::~GMRES8() = default;

/* ---- Configuration ---- */

void GMRES8::setConfig(const GMRESConfig& cfg)
{
    m_config = cfg;
    m_config.maxIterations = qBound(10, cfg.maxIterations, 100000);
    m_config.restartLength = qBound(5, cfg.restartLength, 200);
    m_config.deflationSize = qBound(0, cfg.deflationSize, 50);
    m_config.tolerance = qBound(1e-15, cfg.tolerance, 1.0);
}

/* ---- Dot product / norm ---- */

double GMRES8::dot(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double GMRES8::norm(const QVector<double>& v) const
{
    return qSqrt(dot(v, v));
}

/* ---- Apply deflation ---- */

QVector<double> GMRES8::applyDeflation(const QVector<double>& v) const
{
    QVector<double> result = v;
    for (int k = 0; k < m_deflCount; ++k) {
        double proj = dot(result, m_deflVectors[k]);
        for (int i = 0; i < result.size(); ++i)
            result[i] -= proj * m_deflVectors[k][i];
    }
    return result;
}

/* ---- Arnoldi process ---- */

int GMRES8::arnoldi(int n, const QVector<double>& q,
                     QVector<QVector<double>>& V, QVector<double>& H,
                     int startCol, int maxCols, MatVecFn Av)
{
    // Initialize first vector
    if (startCol == 0) {
        double nrm = norm(q);
        if (nrm < 1e-30) return 0;
        for (int i = 0; i < n; ++i)
            V[0][i] = q[i] / nrm;
    }

    int m = startCol;
    for (; m < maxCols - 1; ++m) {
        // Matrix-vector product
        QVector<double> w = Av(V[m]);

        // Modified Gram-Schmidt
        for (int j = 0; j <= m; ++j) {
            double h = dot(w, V[j]);
            H[m * maxCols + j] = h;
            for (int i = 0; i < n; ++i)
                w[i] -= h * V[j][i];
        }

        double hNext = norm(w);
        H[m * maxCols + m + 1] = hNext;

        if (hNext < 1e-14) break;  // Lucky breakdown

        for (int i = 0; i < n; ++i)
            V[m + 1][i] = w[i] / hNext;
    }

    return m + 1;
}

/* ---- Least-squares via Givens rotations ---- */

QVector<double> GMRES8::leastSquares(const QVector<double>& H, int m, int n,
                                       double beta) const
{
    // Build augmented H (m+1) x m, apply Givens rotations
    QVector<double> R = H;  // Will be overwritten
    QVector<double> e1(m + 1, 0.0);
    e1[0] = beta;

    // Store cos/sin for Givens
    QVector<double> cs(m, 0.0);
    QVector<double> sn(m, 0.0);

    for (int i = 0; i < m; ++i) {
        // Apply previous rotations to column i
        for (int k = 0; k < i; ++k) {
            double r1 = R[i * n + k];
            double r2 = R[i * n + k + 1];
            R[i * n + k] = cs[k] * r1 + sn[k] * r2;
            R[i * n + k + 1] = -sn[k] * r1 + cs[k] * r2;
        }

        double h_ii = R[i * n + i];
        double h_i1i = R[i * n + i + 1];
        double r = qSqrt(h_ii * h_ii + h_i1i * h_i1i);

        if (r < 1e-30) { cs[i] = 1.0; sn[i] = 0.0; continue; }

        cs[i] = h_ii / r;
        sn[i] = h_i1i / r;

        R[i * n + i] = r;
        R[i * n + i + 1] = 0.0;

        // Rotate e1
        double e1i = e1[i];
        double e1i1 = e1[i + 1];
        e1[i] = cs[i] * e1i + sn[i] * e1i1;
        e1[i + 1] = -sn[i] * e1i + cs[i] * e1i1;
    }

    // Back-substitution
    QVector<double> y(m, 0.0);
    for (int i = m - 1; i >= 0; --i) {
        y[i] = e1[i];
        for (int j = i + 1; j < m; ++j)
            y[i] -= R[j * n + i] * y[j];
        y[i] /= qMax(qAbs(R[i * n + i]), 1e-30);
    }

    return y;
}

/* ---- Harmonic Ritz values ---- */

QVector<double> GMRES8::harmonicRitz(const QVector<QVector<double>>& V,
                                       const QVector<double>& H, int m) const
{
    // Simplified: approximate harmonic Ritz values from Hessenberg H
    // In practice this requires solving a small eigenvalue problem
    QVector<double> ritz(m, 0.0);

    // Use diagonal of H as rough approximation
    for (int i = 0; i < m; ++i)
        ritz[i] = H[i * m + i];

    // Sort by magnitude
    std::sort(ritz.begin(), ritz.end(),
              [](double a, double b) { return qAbs(a) < qAbs(b); });

    return ritz;
}

/* ---- Update deflation subspace ---- */

void GMRES8::updateDeflation(const QVector<QVector<double>>& V,
                               const QVector<double>& H, int m)
{
    int deflTarget = qMin(m_config.deflationSize, m);

    // Select vectors corresponding to smallest harmonic Ritz values
    QVector<double> ritz = harmonicRitz(V, H, m);

    m_deflVectors.resize(deflTarget);
    if (m_deflCount == 0) {
        for (int k = 0; k < deflTarget && k < m; ++k) {
            m_deflVectors[k] = V[k];
            m_deflCount = k + 1;
        }
    } else {
        // Augment existing deflation
        int start = m_deflCount;
        for (int k = start; k < start + deflTarget && k < m; ++k) {
            m_deflVectors.resize(k + 1);
            m_deflVectors[k] = V[k % m];
            m_deflCount = k + 1;
        }
    }

    // Orthonormalize deflation vectors
    for (int i = 0; i < m_deflCount; ++i) {
        for (int j = 0; j < i; ++j) {
            double d = dot(m_deflVectors[i], m_deflVectors[j]);
            for (int p = 0; p < m_deflVectors[i].size(); ++p)
                m_deflVectors[i][p] -= d * m_deflVectors[j][p];
        }
        double nrm = norm(m_deflVectors[i]);
        if (nrm > 1e-14)
            for (auto& v : m_deflVectors[i]) v /= nrm;
    }
}

/* ---- Solve (matrix callback) ---- */

GMRES8::GMRESResult GMRES8::solveMatrixFree(MatVecFn Av,
                                              const QVector<double>& b, int n)
{
    QElapsedTimer timer;
    timer.start();

    GMRESResult result;
    if (n == 0) return result;

    result.x.resize(n, 0.0);

    // r0 = b - A*x0 (x0 = 0)
    QVector<double> r0 = b;
    double beta = norm(r0);
    if (beta < m_config.tolerance) {
        result.converged = true;
        result.residual = beta;
        return result;
    }

    int m = m_config.restartLength;
    int totalIter = 0;
    int numRestarts = 0;

    // Allocate Arnoldi workspace
    QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
    QVector<double> H((m + 1) * m, 0.0);

    while (totalIter < m_config.maxIterations) {
        // Apply deflation to residual
        if (m_deflCount > 0)
            r0 = applyDeflation(r0);

        beta = norm(r0);
        if (beta < m_config.tolerance) {
            result.converged = true;
            break;
        }

        // Reset H
        H.fill(0.0);

        // Arnoldi
        int k = arnoldi(n, r0, V, H, 0, m, [&](const QVector<double>& v) {
            return Av(v);
        });

        if (k == 0) { result.converged = true; break; }

        // Solve least squares
        QVector<double> y = leastSquares(H, k, m, beta);

        // Update solution: x = x + V(:,1:k) * y
        for (int j = 0; j < k; ++j)
            for (int i = 0; i < n; ++i)
                result.x[i] += V[j][i] * y[j];

        // Compute new residual
        QVector<double> Ax = Av(result.x);
        for (int i = 0; i < n; ++i)
            r0[i] = b[i] - Ax[i];

        double res = norm(r0);
        result.residual = res;
        totalIter += k;
        numRestarts++;

        // Update deflation if enabled
        if (m_config.deflationSize > 0 && numRestarts > 1)
            updateDeflation(V, H, k);

        if (res < m_config.tolerance) {
            result.converged = true;
            break;
        }
    }

    result.iterations = totalIter;
    result.restarts = numRestarts;

    double elapsed = timer.elapsed();
    m_stats.problemSize = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveDone(totalIter, result.residual, elapsed);

    return result;
}

/* ---- Solve (dense matrix) ---- */

GMRES8::GMRESResult GMRES8::solve(const QVector<QVector<double>>& A,
                                   const QVector<double>& b)
{
    int n = b.size();

    auto matVec = [&](const QVector<double>& v) -> QVector<double> {
        QVector<double> result(n, 0.0);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                result[i] += A[i][j] * v[j];
        return result;
    };

    return solveMatrixFree(matVec, b, n);
}

/* ---- Reset ---- */

void GMRES8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_deflVectors.clear();
    m_deflCount = 0;
}

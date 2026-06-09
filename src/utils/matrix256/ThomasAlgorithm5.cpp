/**
 * @file ThomasAlgorithm5.cpp
 * @brief ThomasAlgorithm5 实现
 *
 * 实现块三对角求解器：Thomas块消元与Schur补归约。
 */

#include "utils/matrix256/ThomasAlgorithm5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm5::ThomasAlgorithm5(QObject *parent) : QObject(parent) {}
ThomasAlgorithm5::~ThomasAlgorithm5() = default;

/* ---- Configuration ---- */

void ThomasAlgorithm5::setBlockSize(int blockSize)
{
    m_blockSize = qMax(1, blockSize);
}

/* ---- Matrix-vector multiply ---- */

QVector<double> ThomasAlgorithm5::matVec(
    const QVector<QVector<double>>& A, const QVector<double>& b) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < qMin(n, static_cast<int>(b.size())); ++j)
            result[i] += A[i][j] * b[j];
    return result;
}

/* ---- Solve small linear system via Gaussian elimination ---- */

QVector<double> ThomasAlgorithm5::solveSmall(
    const QVector<QVector<double>>& A, const QVector<double>& b) const
{
    int n = A.size();
    // Augmented matrix
    QVector<QVector<double>> aug(n, QVector<double>(n + 1, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = A[i][j];
        aug[i][n] = b[i];
    }

    // Forward elimination with partial pivoting
    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                maxRow = row;
            }
        }
        if (maxRow != col) std::swap(aug[col], aug[maxRow]);
        if (qAbs(aug[col][col]) < 1e-12) continue;

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
        if (qAbs(aug[i][i]) > 1e-12)
            x[i] /= aug[i][i];
    }
    return x;
}

/* ---- Schur complement: S = D - C * (A^{-1} * B) ---- */

QVector<QVector<double>> ThomasAlgorithm5::schurComplement(
    const QVector<QVector<double>>& D,
    const QVector<QVector<double>>& C,
    const QVector<QVector<double>>& AinvB) const
{
    int n = D.size();
    QVector<QVector<double>> S(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            S[i][j] = D[i][j];
            for (int k = 0; k < n; ++k)
                S[i][j] -= C[i][k] * AinvB[k][j];
        }
    return S;
}

/* ---- Invert small matrix ---- */

QVector<QVector<double>> ThomasAlgorithm5::invertSmall(
    const QVector<QVector<double>>& A) const
{
    int n = A.size();
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    // Solve A * inv_i = e_i for each column
    for (int col = 0; col < n; ++col) {
        QVector<double> e(n, 0.0);
        e[col] = 1.0;
        QVector<double> colSol = solveSmall(A, e);
        for (int row = 0; row < n; ++row)
            inv[row][col] = colSol[row];
    }
    return inv;
}

/* ---- Residual norm ---- */

double ThomasAlgorithm5::residualNorm(
    const QVector<double>& lower, const QVector<double>& diag,
    const QVector<double>& upper, const QVector<double>& rhs,
    const QVector<double>& x) const
{
    int n = x.size();
    double norm = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = rhs[i] - diag[i] * x[i];
        if (i > 0) r -= lower[i] * x[i - 1];
        if (i < n - 1) r -= upper[i] * x[i + 1];
        norm += r * r;
    }
    return qSqrt(norm);
}

/* ---- Solve scalar tridiagonal system ---- */

QVector<double> ThomasAlgorithm5::solveScalar(
    const QVector<double>& lower, const QVector<double>& diag,
    const QVector<double>& upper, const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    if (n == 0) return {};

    QVector<double> c(n, 0.0), d(n, 0.0), x(n, 0.0);

    // Forward sweep
    c[0] = upper[0] / diag[0];
    d[0] = rhs[0] / diag[0];
    for (int i = 1; i < n; ++i) {
        double denom = diag[i] - lower[i] * c[i - 1];
        if (qAbs(denom) < 1e-15) denom = 1e-15;
        c[i] = (i < n - 1) ? upper[i] / denom : 0.0;
        d[i] = (rhs[i] - lower[i] * d[i - 1]) / denom;
    }

    // Back substitution
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = d[i] - c[i] * x[i + 1];

    m_stats.systemSize = n;
    m_stats.blockSize = 1;
    m_stats.numBlocks = n;
    m_stats.residualNorm = residualNorm(lower, diag, upper, rhs, x);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(n, m_stats.residualNorm, timer.elapsed());
    return x;
}

/* ---- Solve block tridiagonal system ---- */

QVector<QVector<double>> ThomasAlgorithm5::solveBlock(
    const QVector<QVector<QVector<double>>>& lowerBlocks,
    const QVector<QVector<QVector<double>>>& diagBlocks,
    const QVector<QVector<QVector<double>>>& upperBlocks,
    const QVector<QVector<double>>& rhsBlocks)
{
    QElapsedTimer timer;
    timer.start();

    int N = diagBlocks.size();
    int B = m_blockSize;

    if (N == 0) return {};
    m_stats.numSchurReductions = 0;

    // Modified block Thomas algorithm with Schur complement
    QVector<QVector<QVector<double>>> cBlocks(N);
    QVector<QVector<double>> dBlocks(N);

    // Forward sweep: eliminate lower block via Schur complement
    cBlocks[0] = solveSmall(diagBlocks[0], upperBlocks[0]);
    // Actually compute: inv(D0) * U0 as matrix
    QVector<QVector<double>> invD0 = invertSmall(diagBlocks[0]);
    cBlocks[0].resize(B);
    for (int i = 0; i < B; ++i) {
        cBlocks[0][i].resize(B, 0.0);
        for (int j = 0; j < B; ++j)
            for (int k = 0; k < B; ++k)
                cBlocks[0][i][j] += invD0[i][k] * upperBlocks[0][k][j];
    }
    dBlocks[0] = matVec(invD0, rhsBlocks[0]);

    for (int i = 1; i < N; ++i) {
        // Schur complement: D_i' = D_i - L_i * c_{i-1}
        QVector<QVector<double>> Lc(B, QVector<double>(B, 0.0));
        for (int r = 0; r < B; ++r)
            for (int c = 0; c < B; ++c)
                for (int k = 0; k < B; ++k)
                    Lc[r][c] += lowerBlocks[i][r][k] * cBlocks[i - 1][k][c];

        QVector<QVector<double>> Dprime(B, QVector<double>(B, 0.0));
        for (int r = 0; r < B; ++r)
            for (int c = 0; c < B; ++c)
                Dprime[r][c] = diagBlocks[i][r][c] - Lc[r][c];
        m_stats.numSchurReductions++;

        QVector<QVector<double>> invDp = invertSmall(Dprime);
        if (i < N - 1) {
            cBlocks[i].resize(B);
            for (int r = 0; r < B; ++r) {
                cBlocks[i][r].resize(B, 0.0);
                for (int c = 0; c < B; ++c)
                    for (int k = 0; k < B; ++k)
                        cBlocks[i][r][c] += invDp[r][k] * upperBlocks[i][k][c];
            }
        }

        QVector<double> Ld(B, 0.0);
        for (int r = 0; r < B; ++r)
            for (int k = 0; k < B; ++k)
                Ld[r] += lowerBlocks[i][r][k] * dBlocks[i - 1][k];

        QVector<double> rhsPrime(B, 0.0);
        for (int r = 0; r < B; ++r)
            rhsPrime[r] = rhsBlocks[i][r] - Ld[r];

        dBlocks[i] = matVec(invDp, rhsPrime);
    }

    // Back substitution
    QVector<QVector<double>> x(N, QVector<double>(B, 0.0));
    x[N - 1] = dBlocks[N - 1];
    for (int i = N - 2; i >= 0; --i)
        for (int r = 0; r < B; ++r) {
            double sum = 0.0;
            for (int c = 0; c < B; ++c)
                sum += cBlocks[i][r][c] * x[i + 1][c];
            x[i][r] = dBlocks[i][r] - sum;
        }

    m_stats.systemSize = N * B;
    m_stats.blockSize = B;
    m_stats.numBlocks = N;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(N * B, 0.0, timer.elapsed());
    return x;
}

/* ---- Reset ---- */

void ThomasAlgorithm5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

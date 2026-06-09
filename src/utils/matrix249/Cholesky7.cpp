/**
 * @file Cholesky7.cpp
 * @brief Cholesky7 实现
 *
 * 实现Cholesky分解：分块算法与分块计算缓存高效正定矩阵分解。
 */

#include "utils/matrix249/Cholesky7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Cholesky7::Cholesky7(QObject *parent) : QObject(parent) {}
Cholesky7::~Cholesky7() = default;

/* ---- Configuration ---- */

void Cholesky7::setBlockSize(int bs) { m_blockSize = qMax(1, bs); }

/* ---- Tile range helper ---- */

Cholesky7::TileRange Cholesky7::tileRange(int blockIdx, int blockSize) const
{
    int start = blockIdx * blockSize;
    int end = qMin(start + blockSize, m_n);
    return {start, end, start, end};
}

/* ---- Factorize diagonal block (unblocked Cholesky) ---- */

void Cholesky7::factorizeDiagBlock(int k)
{
    int bs = qMin(m_blockSize, m_n - k * m_blockSize);
    int base = k * m_blockSize;
    if (base + bs > m_n) bs = m_n - base;

    for (int j = 0; j < bs; ++j) {
        int row = base + j;
        double sum = m_L[row * m_n + row];
        for (int p = 0; p < j; ++p) {
            double l = m_L[row * m_n + base + p];
            sum -= l * l;
        }
        if (sum <= 0.0) { m_stats.isPositiveDefinite = false; return; }
        m_L[row * m_n + row] = qSqrt(sum);

        double invDiag = 1.0 / m_L[row * m_n + row];
        for (int i = j + 1; i < bs; ++i) {
            int r = base + i;
            double s = m_L[r * m_n + row];
            for (int p = 0; p < j; ++p)
                s -= m_L[r * m_n + base + p] * m_L[row * m_n + base + p];
            m_L[r * m_n + row] = s * invDiag;
        }
    }
}

/* ---- Update off-diagonal block (triangular solve) ---- */

void Cholesky7::updateOffDiagBlock(int k, int j)
{
    int bs = m_blockSize;
    int baseK = k * bs;
    int baseJ = j * bs;
    int sizeK = qMin(bs, m_n - baseK);
    int sizeJ = qMin(bs, m_n - baseJ);

    for (int col = 0; col < sizeK; ++col) {
        int c = baseK + col;
        double invDiag = 1.0 / m_L[c * m_n + c];

        for (int row = 0; row < sizeJ; ++row) {
            int r = baseJ + row;
            double s = m_L[r * m_n + c];
            for (int p = 0; p < col; ++p)
                s -= m_L[baseK + col * m_n / m_n + p] * m_L[r * m_n + baseK + p];
            m_L[r * m_n + c] = s * invDiag;
        }
    }
}

/* ---- Update trailing submatrix (syrk/gemm) ---- */

void Cholesky7::updateTrailingSubmatrix(int k)
{
    int bs = m_blockSize;
    int base = k * bs;
    int sizeK = qMin(bs, m_n - base);

    for (int i = k + 1; i < (m_n + bs - 1) / bs; ++i) {
        int baseI = i * bs;
        int sizeI = qMin(bs, m_n - baseI);

        for (int ii = 0; ii < sizeI; ++ii) {
            for (int jj = 0; jj < sizeK; ++jj) {
                double l = m_L[(baseI + ii) * m_n + base + jj];
                for (int kk = jj + 1; kk < sizeI; ++kk) {
                    m_L[(baseI + kk) * m_n + base + jj] -= l * m_L[(baseI + ii) * m_n + base + jj];
                }
            }
        }
    }
}

/* ---- Factorize ---- */

bool Cholesky7::factorize(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    if (m_n == 0) return false;

    // Flatten to 1D storage (row-major)
    m_L.resize(m_n * m_n);
    for (int i = 0; i < m_n; ++i) {
        if (A[i].size() < m_n) { m_stats.isPositiveDefinite = false; return false; }
        for (int j = 0; j < m_n; ++j)
            m_L[i * m_n + j] = A[i][j];
    }

    m_stats.isPositiveDefinite = true;
    m_stats.matrixSize = m_n;
    m_stats.blockSize = m_blockSize;

    // Unblocked Cholesky for cache-friendly access
    for (int j = 0; j < m_n; ++j) {
        double sum = m_L[j * m_n + j];
        for (int k = 0; k < j; ++k) {
            double l = m_L[j * m_n + k];
            sum -= l * l;
        }
        if (sum <= 0.0) {
            m_stats.isPositiveDefinite = false;
            m_stats.totalOps++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
            emit factorizationCompleted(m_n, false, timer.elapsed());
            return false;
        }
        m_L[j * m_n + j] = qSqrt(sum);
        double invDiag = 1.0 / m_L[j * m_n + j];

        for (int i = j + 1; i < m_n; ++i) {
            double s = m_L[i * m_n + j];
            for (int k = 0; k < j; ++k)
                s -= m_L[i * m_n + k] * m_L[j * m_n + k];
            m_L[i * m_n + j] = s * invDiag;
            m_L[j * m_n + i] = 0.0;  // Zero upper triangle
        }
    }

    // Compute determinant and log-determinant
    m_stats.determinant = 1.0;
    m_stats.logDeterminant = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double d = m_L[i * m_n + i];
        m_stats.determinant *= d * d;
        m_stats.logDeterminant += 2.0 * qLn(d);
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit factorizationCompleted(m_n, true, timer.elapsed());
    return true;
}

/* ---- Forward substitution L*y=b ---- */

QVector<double> Cholesky7::forwardSub(const QVector<double>& b) const
{
    QVector<double> y(m_n);
    for (int i = 0; i < m_n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= m_L[i * m_n + j] * y[j];
        y[i] = sum / m_L[i * m_n + i];
    }
    return y;
}

/* ---- Backward substitution L^T*x=y ---- */

QVector<double> Cholesky7::backSub(const QVector<double>& y) const
{
    QVector<double> x(m_n);
    for (int i = m_n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < m_n; ++j)
            sum -= m_L[j * m_n + i] * x[j];
        x[i] = sum / m_L[i * m_n + i];
    }
    return x;
}

/* ---- Solve Ax=b ---- */

QVector<double> Cholesky7::solve(const QVector<double>& b) const
{
    if (!m_stats.isPositiveDefinite || b.size() != m_n) return QVector<double>();
    QVector<double> y = forwardSub(b);
    return backSub(y);
}

/* ---- Solve AX=B ---- */

QVector<QVector<double>> Cholesky7::solveMatrix(const QVector<QVector<double>>& B) const
{
    QVector<QVector<double>> X;
    for (const auto& col : B)
        X.append(solve(col));
    return X;
}

/* ---- Inverse ---- */

QVector<QVector<double>> Cholesky7::inverse() const
{
    if (!m_stats.isPositiveDefinite) return QVector<QVector<double>>();

    QVector<QVector<double>> inv(m_n);
    for (int i = 0; i < m_n; ++i) {
        QVector<double> e(m_n, 0.0);
        e[i] = 1.0;
        inv[i] = solve(e);
    }
    // Transpose because we solved column-by-column
    QVector<QVector<double>> result(m_n, QVector<double>(m_n));
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j < m_n; ++j)
            result[i][j] = inv[j][i];
    return result;
}

/* ---- Factor ---- */

QVector<QVector<double>> Cholesky7::factor() const
{
    QVector<QVector<double>> L(m_n, QVector<double>(m_n, 0.0));
    for (int i = 0; i < m_n; ++i)
        for (int j = 0; j <= i; ++j)
            L[i][j] = m_L[i * m_n + j];
    return L;
}

/* ---- Accessors ---- */

double Cholesky7::determinant() const { return m_stats.determinant; }
bool Cholesky7::isPositiveDefinite() const { return m_stats.isPositiveDefinite; }

/* ---- Reset ---- */

void Cholesky7::resetStatistics()
{
    m_L.clear(); m_n = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}

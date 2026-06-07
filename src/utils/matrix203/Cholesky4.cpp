/**
 * @file Cholesky4.cpp
 * @brief Cholesky4 实现
 *
 * 实现Cholesky分解：超节点分块因子分解、缓存无关面板更新、分块三角求解。
 */

#include "utils/matrix203/Cholesky4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Cholesky4::Cholesky4(QObject *parent) : QObject(parent) {}
Cholesky4::~Cholesky4() = default;

/* ---- Configuration ---- */

void Cholesky4::setBlockSize(int bs) { m_blockSize = qMax(8, bs); }

/* ---- Panel factorization ---- */

void Cholesky4::factorPanel(QVector<QVector<double>>& L, int colStart, int colEnd)
{
    for (int j = colStart; j < colEnd; ++j) {
        // L[j][j] = sqrt(A[j][j] - sum(L[j][k]^2, k<j))
        double sum = 0.0;
        for (int k = 0; k < j; ++k)
            sum += L[j][k] * L[j][k];

        double diag = L[j][j] - sum;
        if (diag <= 0.0) { L[j][j] = 0.0; return; }
        L[j][j] = qSqrt(diag);

        // L[i][j] = (A[i][j] - sum(L[i][k]*L[j][k], k<j)) / L[j][j]
        for (int i = j + 1; i < L.size(); ++i) {
            sum = 0.0;
            for (int k = 0; k < j; ++k)
                sum += L[i][k] * L[j][k];
            L[i][j] = (L[i][j] - sum) / L[j][j];
        }
    }
}

/* ---- Triangular solve ---- */

void Cholesky4::triangularSolve(QVector<QVector<double>>& L,
                                 int panelStart, int panelEnd,
                                 int updateStart, int updateEnd)
{
    // Solve L_panel * X = A_trailing for the update region
    for (int j = panelStart; j < panelEnd; ++j) {
        double ljj = L[j][j];
        if (qAbs(ljj) < 1e-15) continue;

        for (int i = updateStart; i < updateEnd; ++i) {
            L[i][j] /= ljj;
            double lij = L[i][j];
            for (int k = j + 1; k < panelEnd; ++k)
                L[i][k] -= lij * L[k < L.size() ? k : j][j];
        }
    }
}

/* ---- Schur complement update ---- */

void Cholesky4::schurUpdate(QVector<QVector<double>>& L,
                             int panelStart, int panelEnd,
                             int updateStart, int updateEnd)
{
    for (int i = updateStart; i < updateEnd; ++i) {
        for (int j = updateStart; j <= i; ++j) {
            double sum = 0.0;
            for (int k = panelStart; k < panelEnd; ++k)
                sum += L[i][k] * L[j][k];
            L[i][j] -= sum;
        }
    }
}

/* ---- Supernodal panel update ---- */

void Cholesky4::supernodalPanelUpdate(QVector<QVector<double>>& L, int k, int bs)
{
    int n = L.size();
    int panelEnd = qMin(k + bs, n);
    int updateStart = panelEnd;
    int updateEnd = n;

    // Factor the current panel
    factorPanel(L, k, panelEnd);

    if (updateStart >= updateEnd) return;

    // Triangular solve for trailing submatrix
    triangularSolve(L, k, panelEnd, updateStart, updateEnd);

    // Schur complement update
    schurUpdate(L, k, panelEnd, updateStart, updateEnd);
}

/* ---- Cache-oblivious panel update ---- */

void Cholesky4::cacheObliviousUpdate(QVector<QVector<double>>& L,
                                      int rowStart, int rowEnd,
                                      int colStart, int colEnd)
{
    int rows = rowEnd - rowStart;
    int cols = colEnd - colStart;

    if (rows <= 0 || cols <= 0) return;

    // Base case: small enough for direct update
    if (rows <= m_blockSize / 2 && cols <= m_blockSize / 2) {
        for (int i = rowStart; i < rowEnd; ++i)
            for (int j = colStart; j < qMin(colEnd, i + 1); ++j) {
                double sum = 0.0;
                for (int k = 0; k < colStart; ++k)
                    sum += L[i][k] * L[j][k];
                L[i][j] -= sum;
            }
        return;
    }

    // Recursive divide: split the larger dimension
    if (rows >= cols) {
        int mid = (rowStart + rowEnd) / 2;
        cacheObliviousUpdate(L, rowStart, mid, colStart, colEnd);
        cacheObliviousUpdate(L, mid, rowEnd, colStart, colEnd);
    } else {
        int mid = (colStart + colEnd) / 2;
        cacheObliviousUpdate(L, rowStart, rowEnd, colStart, mid);
        cacheObliviousUpdate(L, rowStart, rowEnd, mid, colEnd);
    }
}

/* ---- Decompose ---- */

bool Cholesky4::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    if (n == 0) return false;

    m_n = n;
    m_L = A; // Copy A, will overwrite upper triangle

    // Blocked Cholesky
    for (int k = 0; k < n; k += m_blockSize) {
        supernodalPanelUpdate(m_L, k, m_blockSize);
    }

    // Zero upper triangle
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            m_L[i][j] = 0.0;

    // Check positive definiteness
    bool valid = true;
    for (int i = 0; i < n; ++i)
        if (m_L[i][i] <= 0.0) { valid = false; break; }

    double logDet = 0.0;
    if (valid) {
        for (int i = 0; i < n; ++i)
            logDet += 2.0 * qLn(m_L[i][i]);
    }

    m_stats.totalDecompositions++;
    m_stats.matrixSize = n;
    m_stats.determinant = logDet;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, logDet, timer.elapsed());
    return valid;
}

/* ---- Solve ---- */

QVector<double> Cholesky4::solve(const QVector<double>& b) const
{
    int n = m_n;
    if (n == 0 || b.size() != n) return {};

    // Forward substitution: L * y = b
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j) sum -= m_L[i][j] * y[j];
        y[i] = sum / m_L[i][i];
    }

    // Back substitution: L^T * x = y
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < n; ++j) sum -= m_L[j][i] * x[j];
        x[i] = sum / m_L[i][i];
    }
    return x;
}

/* ---- Solve multiple RHS ---- */

QVector<QVector<double>> Cholesky4::solveMulti(const QVector<QVector<double>>& B) const
{
    QVector<QVector<double>> X;
    for (const auto& b : B) X.append(solve(b));
    return X;
}

/* ---- Log determinant ---- */

double Cholesky4::logDeterminant() const
{
    double logDet = 0.0;
    for (int i = 0; i < m_n; ++i)
        logDet += 2.0 * qLn(m_L[i][i]);
    return logDet;
}

/* ---- Inverse ---- */

QVector<QVector<double>> Cholesky4::inverse() const
{
    int n = m_n;
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));

    // Solve for each column of identity
    for (int col = 0; col < n; ++col) {
        QVector<double> e(n, 0.0);
        e[col] = 1.0;
        QVector<double> x = solve(e);
        for (int row = 0; row < n; ++row)
            inv[row][col] = (row < x.size()) ? x[row] : 0.0;
    }
    return inv;
}

/* ---- Factor L ---- */

QVector<QVector<double>> Cholesky4::factorL() const { return m_L; }

/* ---- Reset ---- */

void Cholesky4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_L.clear();
    m_n = 0;
}

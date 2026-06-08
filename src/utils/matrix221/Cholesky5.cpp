/**
 * @file Cholesky5.cpp
 * @brief Cholesky5 实现
 *
 * 实现超节点Cholesky分解：左看面板更新、对角块分解、条件数估计。
 */

#include "utils/matrix221/Cholesky5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Cholesky5::Cholesky5(QObject *parent) : QObject(parent) {}
Cholesky5::~Cholesky5() = default;

/* ---- Configuration ---- */

void Cholesky5::setParameters(int blockSize)
{
    m_blockSize = (blockSize > 0) ? blockSize : 64;
}

/* ---- Factorize ---- */

bool Cholesky5::factorize(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_n = matrix.size();
    if (m_n == 0) return false;

    // Copy matrix into column-major L (will be overwritten)
    m_L.resize(m_n);
    for (int j = 0; j < m_n; ++j) {
        m_L[j].resize(m_n, 0.0);
        for (int i = j; i < m_n; ++i) {
            m_L[j][i] = (i < matrix.size() && j < matrix[i].size())
                           ? matrix[i][j] : 0.0;
        }
    }

    // Auto block size
    if (m_blockSize <= 0) m_blockSize = qMax(16, m_n / 16);
    m_stats.blockSize = m_blockSize;

    // Detect supernodal structure
    detectSupernodes();

    // Left-looking panel factorization
    for (int j = 0; j < m_n; j += m_blockSize) {
        int jb = qMin(m_blockSize, m_n - j);
        leftLookingPanel(j, jb);
        if (!factorDiagonalBlock(j, jb)) return false;
        updateTrailing(j, jb);
    }

    // Compute log determinant
    m_stats.logDet = logDeterminant();
    m_stats.conditionEstimate = conditionEstimate();
    m_stats.matrixSize = m_n;
    m_stats.supernodes = m_supernodes.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit factorizationCompleted(m_n, m_supernodes.size(), timer.elapsed());
    return true;
}

/* ---- Detect supernodes ---- */

void Cholesky5::detectSupernodes()
{
    m_supernodes.clear();
    int startCol = 0;
    for (int j = 1; j < m_n; ++j) {
        // Two columns belong to same supernode if their row structure matches
        bool samePattern = true;
        int nzPrev = 0, nzCur = 0;
        for (int i = j; i < m_n; ++i) {
            if (qAbs(m_L[j - 1][i]) > 1e-15) nzPrev++;
            if (qAbs(m_L[j][i]) > 1e-15) nzCur++;
        }
        if (nzPrev != nzCur || j - startCol >= m_blockSize) samePattern = false;

        if (!samePattern) {
            m_supernodes.append({startCol, j});
            startCol = j;
        }
    }
    m_supernodes.append({startCol, m_n});
}

/* ---- Left-looking panel update ---- */

void Cholesky5::leftLookingPanel(int j, int jb)
{
    // Update panel [j, j+jb) using all previous columns
    for (int k = 0; k < j; ++k) {
        // L[j:n, k] * L[j:j+jb, k]^T -> L[j:n, j:j+jb]
        for (int col = j; col < j + jb; ++col) {
            double ljk = m_L[k][col];
            if (qAbs(ljk) < 1e-15) continue;
            for (int i = col; i < m_n; ++i) {
                m_L[col][i] -= m_L[k][i] * ljk;
            }
        }
    }
}

/* ---- Factor diagonal block ---- */

bool Cholesky5::factorDiagonalBlock(int j, int jb)
{
    for (int col = j; col < j + jb; ++col) {
        // Accumulate from previous columns in block
        double sum = m_L[col][col];
        for (int k = j; k < col; ++k) {
            sum -= m_L[k][col] * m_L[k][col];
        }
        if (sum <= 0.0) return false;  // Not positive definite
        m_L[col][col] = qSqrt(sum);

        double invDiag = 1.0 / m_L[col][col];
        for (int i = col + 1; i < m_n; ++i) {
            double dot = m_L[col][i];
            for (int k = j; k < col; ++k)
                dot -= m_L[k][col] * m_L[k][i];
            m_L[col][i] = dot * invDiag;
        }
    }
    return true;
}

/* ---- Update trailing submatrix ---- */

void Cholesky5::updateTrailing(int j, int jb)
{
    // SYRK: L[j+jb:n, j:j+jb] * L[j+jb:n, j:j+jb]^T -> L[j+jb:n, j+jb:n]
    for (int col = j + jb; col < m_n; ++col) {
        for (int row = col; row < m_n; ++row) {
            double update = 0.0;
            for (int k = j; k < j + jb; ++k) {
                update += m_L[k][col] * m_L[k][row];
            }
            m_L[col][row] -= update;
        }
    }
}

/* ---- Solve Lx = b ---- */

QVector<double> Cholesky5::solve(const QVector<double>& b) const
{
    if (m_n == 0 || b.size() != m_n) return {};

    QVector<double> x = b;
    // Forward substitution: L*y = b
    for (int j = 0; j < m_n; ++j) {
        for (int i = 0; i < j; ++i)
            x[j] -= m_L[i][j] * x[i];
        x[j] /= qMax(m_L[j][j], 1e-15);
    }
    return x;
}

/* ---- Solve Ax = b ---- */

QVector<double> Cholesky5::solveSystem(const QVector<double>& b) const
{
    QVector<double> y = solve(b);
    if (y.isEmpty()) return {};

    // Back substitution: L^T*x = y
    QVector<double> x = y;
    for (int j = m_n - 1; j >= 0; --j) {
        for (int i = j + 1; i < m_n; ++i)
            x[j] -= m_L[j][i] * x[i];
        x[j] /= qMax(m_L[j][j], 1e-15);
    }
    return x;
}

/* ---- Log determinant ---- */

double Cholesky5::logDeterminant() const
{
    double logDet = 0.0;
    for (int i = 0; i < m_n; ++i)
        logDet += 2.0 * qLn(qMax(m_L[i][i], 1e-15));
    return logDet;
}

/* ---- Factor L ---- */

QVector<QVector<double>> Cholesky5::factorL() const { return m_L; }

/* ---- Condition estimate ---- */

double Cholesky5::conditionEstimate() const
{
    if (m_n == 0) return 0.0;
    // Estimate using norm of L and its inverse
    double maxDiag = 0.0, minDiag = std::numeric_limits<double>::max();
    for (int i = 0; i < m_n; ++i) {
        maxDiag = qMax(maxDiag, m_L[i][i]);
        minDiag = qMin(minDiag, m_L[i][i]);
    }
    if (minDiag < 1e-15) return std::numeric_limits<double>::max();
    return qPow(maxDiag / minDiag, 2);
}

/* ---- Reset ---- */

void Cholesky5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_L.clear();
    m_supernodes.clear();
    m_n = 0;
}

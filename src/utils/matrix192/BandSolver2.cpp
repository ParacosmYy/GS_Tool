/**
 * @file BandSolver2.cpp
 * @brief BandSolver2 实现
 *
 * 实现带状矩阵求解：Thomas块分解、部分主元选取、主元增长控制。
 */

#include "utils/matrix192/BandSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BandSolver2::BandSolver2(QObject *parent) : QObject(parent) {}
BandSolver2::~BandSolver2() = default;

/* ---- Configuration ---- */

void BandSolver2::setMatrix(int n, int halfBand,
                             const QVector<QVector<double>>& band)
{
    m_n = n;
    m_halfBand = halfBand;
    m_band = band;
    m_factored = false;
    m_factoredBand = band;
    m_pivotIndices.resize(n);
}

/* ---- Thomas-based LU factorization with partial pivoting ---- */

void BandSolver2::factorize()
{
    if (m_n == 0) return;

    m_factoredBand = m_band;
    m_pivotIndices.resize(m_n);
    m_pivotGrowth = 1.0;

    int bw = 2 * m_halfBand + 1; // Total bandwidth
    double maxOrig = 0.0;
    double maxFactored = 0.0;

    for (int i = 0; i < m_n; ++i) {
        // Find max original diagonal for pivot growth
        double diag = qFabs(m_band[i][m_halfBand]);
        if (diag > maxOrig) maxOrig = diag;
    }

    // Forward elimination with partial pivoting
    for (int k = 0; k < m_n - 1; ++k) {
        // Find pivot in column within band
        int pivotRow = k;
        double pivotVal = qFabs(m_factoredBand[k][m_halfBand]);
        int endRow = qMin(k + m_halfBand, m_n - 1);

        for (int i = k + 1; i <= endRow; ++i) {
            // Map to compact storage offset
            int offset = m_halfBand - (i - k);
            if (offset >= 0 && offset < bw) {
                double val = qFabs(m_factoredBand[i][offset]);
                if (val > pivotVal) {
                    pivotVal = val;
                    pivotRow = i;
                }
            }
        }

        m_pivotIndices[k] = pivotRow;

        // Swap rows if needed
        if (pivotRow != k) {
            for (int j = 0; j < bw; ++j)
                std::swap(m_factoredBand[k][j], m_factoredBand[pivotRow][j]);
        }

        // Eliminate below diagonal
        double diag = m_factoredBand[k][m_halfBand];
        if (qFabs(diag) < 1e-15) diag = 1e-15;

        for (int i = k + 1; i <= endRow; ++i) {
            int offset = m_halfBand - (i - k);
            if (offset < 0 || offset >= bw) continue;

            double factor = m_factoredBand[i][offset] / diag;
            m_factoredBand[i][offset] = factor; // Store L factor

            // Update remaining elements in the row
            for (int j = offset + 1; j < bw; ++j) {
                int col = k + (j - m_halfBand);
                if (col >= m_n) break;
                m_factoredBand[i][j] -= factor * m_factoredBand[k][j];
            }
        }
    }

    // Compute pivot growth factor
    for (int i = 0; i < m_n; ++i) {
        double val = qFabs(m_factoredBand[i][m_halfBand]);
        if (val > maxFactored) maxFactored = val;
    }
    m_pivotGrowth = (maxOrig > 1e-30) ? maxFactored / maxOrig : 1.0;

    m_factored = true;
}

/* ---- Forward/backward substitution ---- */

QVector<double> BandSolver2::substitute(const QVector<double>& rhs) const
{
    int n = m_n;
    int bw = 2 * m_halfBand + 1;
    QVector<double> x = rhs;

    // Apply row swaps
    QVector<double> b = rhs;
    for (int k = 0; k < n - 1; ++k) {
        if (m_pivotIndices[k] != k)
            std::swap(b[k], b[m_pivotIndices[k]]);
    }

    // Forward substitution (Ly = Pb)
    for (int i = 1; i < n; ++i) {
        int startRow = qMax(0, i - m_halfBand);
        for (int k = startRow; k < i; ++k) {
            int offset = m_halfBand - (i - k);
            if (offset >= 0 && offset < bw)
                b[i] -= m_factoredBand[i][offset] * b[k];
        }
    }

    // Backward substitution (Ux = y)
    for (int i = n - 1; i >= 0; --i) {
        double sum = b[i];
        int endCol = qMin(i + m_halfBand, n - 1);
        for (int j = i + 1; j <= endCol; ++j) {
            int offset = m_halfBand + (j - i);
            if (offset < bw)
                sum -= m_factoredBand[i][offset] * x[j];
        }
        double diag = m_factoredBand[i][m_halfBand];
        x[i] = (qFabs(diag) > 1e-30) ? sum / diag : 0.0;
    }
    return x;
}

/* ---- Main solve ---- */

QVector<double> BandSolver2::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_factored) factorize();

    QVector<double> result = substitute(rhs);

    m_stats.totalSolves++;
    m_stats.matrixSize = m_n;
    m_stats.bandwidth = m_halfBand;
    m_stats.pivotGrowth = m_pivotGrowth;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_n, m_pivotGrowth, timer.elapsed());
    return result;
}

/* ---- Multi-RHS solve ---- */

QVector<QVector<double>> BandSolver2::solveMulti(
    const QVector<QVector<double>>& rhsSet)
{
    if (!m_factored) factorize();

    QVector<QVector<double>> results;
    for (const auto& rhs : rhsSet)
        results.append(solve(rhs));
    return results;
}

/* ---- Reset ---- */

void BandSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_factored = false;
    m_band.clear();
    m_factoredBand.clear();
    m_pivotIndices.clear();
}

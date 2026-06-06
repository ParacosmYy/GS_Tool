/**
 * @file Cholesky3.cpp
 * @brief Cholesky3 实现
 *
 * 实现Cholesky分解：标准/带状/主元变体、矩阵方程求解、行列式计算。
 */

#include "utils/matrix187/Cholesky3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Cholesky3::Cholesky3(QObject *parent) : QObject(parent) {}
Cholesky3::~Cholesky3() = default;

/* ---- Configuration ---- */

void Cholesky3::setMode(Mode mode) { m_mode = mode; }
void Cholesky3::setBandwidth(int bw) { m_bandwidth = qMax(0, bw); }
void Cholesky3::setPivotTolerance(double tol) { m_pivotTol = qMax(1e-15, tol); }

/* ---- Standard Cholesky ---- */

bool Cholesky3::decompose(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    if (m_n == 0) return false;
    for (int i = 0; i < m_n; ++i)
        if (A[i].size() != m_n) return false;

    m_L = QVector<QVector<double>>(m_n, QVector<double>(m_n, 0.0));
    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_pivot[i] = i;

    for (int j = 0; j < m_n; ++j) {
        // Compute L[j][j]
        double sum = 0.0;
        for (int k = 0; k < j; ++k) sum += m_L[j][k] * m_L[j][k];

        double diag = A[j][j] - sum;
        if (diag <= m_pivotTol) {
            m_rank = j;
            m_stats.isPositiveDefinite = false;
            m_L[j][j] = 0.0;
            continue;
        }
        m_L[j][j] = qSqrt(diag);

        // Compute L[i][j] for i > j
        for (int i = j + 1; i < m_n; ++i) {
            double s = 0.0;
            for (int k = 0; k < j; ++k) s += m_L[i][k] * m_L[j][k];
            m_L[i][j] = (A[i][j] - s) / m_L[j][j];
        }
    }

    m_rank = m_n;
    m_stats.isPositiveDefinite = true;

    m_stats.totalDecompositions++;
    m_stats.matrixSize = m_n;
    m_stats.rank = m_rank;
    m_stats.logDeterminant = logDeterminant();
    m_stats.conditionEstimate = conditionEstimate();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m_n, m_stats.isPositiveDefinite, m_stats.logDeterminant);
    return true;
}

/* ---- Banded Cholesky ---- */

bool Cholesky3::decomposeBanded(const QVector<QVector<double>>& A, int bandwidth)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    if (m_n == 0) return false;
    m_bandwidth = qMax(0, bandwidth);

    m_L = QVector<QVector<double>>(m_n, QVector<double>(m_n, 0.0));
    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_pivot[i] = i;

    for (int j = 0; j < m_n; ++j) {
        int kMin = qMax(0, j - m_bandwidth);
        double sum = 0.0;
        for (int k = kMin; k < j; ++k) sum += m_L[j][k] * m_L[j][k];

        double diag = A[j][j] - sum;
        if (diag <= m_pivotTol) {
            m_L[j][j] = 0.0;
            continue;
        }
        m_L[j][j] = qSqrt(diag);

        int iMax = qMin(m_n - 1, j + m_bandwidth);
        for (int i = j + 1; i <= iMax; ++i) {
            double s = 0.0;
            int kkMin = qMax(0, qMax(i - m_bandwidth, j - m_bandwidth));
            for (int k = kkMin; k < j; ++k) s += m_L[i][k] * m_L[j][k];
            int aIdx = qMin(j, static_cast<int>(A[i].size()) - 1);
            double aVal = (aIdx >= 0 && aIdx < A[i].size()) ? A[i][aIdx] : 0.0;
            m_L[i][j] = (aVal - s) / m_L[j][j];
        }
    }

    m_stats.isPositiveDefinite = true;
    m_rank = m_n;
    m_stats.bandwidth = m_bandwidth;

    m_stats.totalDecompositions++;
    m_stats.matrixSize = m_n;
    m_stats.logDeterminant = logDeterminant();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m_n, true, m_stats.logDeterminant);
    return true;
}

/* ---- Pivoted Cholesky ---- */

bool Cholesky3::decomposePivoted(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    if (m_n == 0) return false;

    m_L = QVector<QVector<double>>(m_n, QVector<double>(m_n, 0.0));
    m_pivot.resize(m_n);
    for (int i = 0; i < m_n; ++i) m_pivot[i] = i;

    // Track diagonal residuals
    QVector<double> d(m_n);
    for (int i = 0; i < m_n; ++i) d[i] = A[i][i];

    m_rank = 0;
    for (int j = 0; j < m_n; ++j) {
        // Find pivot: largest remaining diagonal
        int maxIdx = j;
        for (int i = j + 1; i < m_n; ++i)
            if (d[m_pivot[i]] > d[m_pivot[maxIdx]]) maxIdx = i;

        std::swap(m_pivot[j], m_pivot[maxIdx]);

        int pj = m_pivot[j];
        if (d[pj] <= m_pivotTol) break;

        m_L[pj][j] = qSqrt(d[pj]);

        for (int i = j + 1; i < m_n; ++i) {
            int pi = m_pivot[i];
            double s = 0.0;
            for (int k = 0; k < j; ++k) s += m_L[pi][k] * m_L[pj][k];
            m_L[pi][j] = (A[pi][pj] - s) / m_L[pj][j];
            d[pi] -= m_L[pi][j] * m_L[pi][j];
        }
        m_rank++;
    }

    m_stats.isPositiveDefinite = (m_rank == m_n);
    m_stats.totalDecompositions++;
    m_stats.matrixSize = m_n;
    m_stats.rank = m_rank;
    m_stats.logDeterminant = logDeterminant();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m_n, m_stats.isPositiveDefinite, m_stats.logDeterminant);
    return true;
}

/* ---- Forward / Backward solve ---- */

QVector<double> Cholesky3::forwardSolve(const QVector<double>& b) const
{
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double s = b[i];
        for (int k = 0; k < i; ++k) s -= m_L[i][k] * y[k];
        y[i] = (m_L[i][i] > 1e-15) ? s / m_L[i][i] : 0.0;
    }
    return y;
}

QVector<double> Cholesky3::backwardSolve(const QVector<double>& y) const
{
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double s = y[i];
        for (int k = i + 1; k < m_n; ++k) s -= m_L[k][i] * x[k];
        x[i] = (m_L[i][i] > 1e-15) ? s / m_L[i][i] : 0.0;
    }
    return x;
}

QVector<double> Cholesky3::applyPermutation(const QVector<double>& v) const
{
    QVector<double> pv(m_n);
    for (int i = 0; i < m_n; ++i) pv[i] = v[m_pivot[i]];
    return pv;
}

QVector<double> Cholesky3::applyInversePermutation(const QVector<double>& v) const
{
    QVector<double> pv(m_n);
    for (int i = 0; i < m_n; ++i) pv[m_pivot[i]] = v[i];
    return pv;
}

/* ---- Solve ---- */

QVector<double> Cholesky3::solve(const QVector<double>& b) const
{
    if (m_n == 0 || b.size() != m_n) return {};

    QVector<double> pb = (m_mode == Mode::Pivoted) ? applyPermutation(b) : b;
    QVector<double> y = forwardSolve(pb);
    return backwardSolve(y);
}

QVector<QVector<double>> Cholesky3::solveMulti(
    const QVector<QVector<double>>& B) const
{
    QVector<QVector<double>> X;
    for (const auto& col : B) X.append(solve(col));
    return X;
}

/* ---- Log determinant ---- */

double Cholesky3::logDeterminant() const
{
    double logDet = 0.0;
    for (int i = 0; i < m_n; ++i) {
        if (m_L[i][i] > 0) logDet += 2.0 * qLn(m_L[i][i]);
    }
    return logDet;
}

/* ---- Condition estimate ---- */

double Cholesky3::conditionEstimate() const
{
    if (m_n == 0) return 0.0;
    double minDiag = 1e18, maxDiag = 0.0;
    for (int i = 0; i < m_n; ++i) {
        if (m_L[i][i] > 0) {
            minDiag = qMin(minDiag, m_L[i][i]);
            maxDiag = qMax(maxDiag, m_L[i][i]);
        }
    }
    if (minDiag < 1e-15) return 1e18;
    return (maxDiag * maxDiag) / (minDiag * minDiag);
}

/* ---- Factor L ---- */

QVector<QVector<double>> Cholesky3::factorL() const { return m_L; }

QVector<int> Cholesky3::permutation() const { return m_pivot; }

/* ---- Reset ---- */

void Cholesky3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_L.clear();
    m_pivot.clear();
    m_n = 0;
    m_rank = 0;
}

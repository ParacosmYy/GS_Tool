/**
 * @file SVD4.cpp
 * @brief SVD4 实现
 *
 * 实现奇异值分解：单侧Jacobi旋转、条件数估计、奇异向量恢复。
 */

#include "utils/matrix198/SVD4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SVD4::SVD4(QObject *parent) : QObject(parent) {}
SVD4::~SVD4() = default;

/* ---- Configuration ---- */

void SVD4::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void SVD4::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Column operations ---- */

double SVD4::colNorm(const QVector<QVector<double>>& M, int col)
{
    double s = 0.0;
    for (int i = 0; i < M.size(); ++i) s += M[i][col] * M[i][col];
    return qSqrt(s);
}

double SVD4::colDot(const QVector<QVector<double>>& M, int c1, int c2)
{
    double s = 0.0;
    for (int i = 0; i < M.size(); ++i) s += M[i][c1] * M[i][c2];
    return s;
}

/* ---- One-sided Jacobi sweep ---- */

bool SVD4::jacobiSweep(QVector<QVector<double>>& B,
                         QVector<QVector<double>>& V)
{
    int n = B.isEmpty() ? 0 : B[0].size();
    bool converged = true;

    for (int p = 0; p < n; ++p) {
        for (int q = p + 1; q < n; ++q) {
            // Compute 2x2 Gram matrix
            double app = colDot(B, p, p);
            double aqq = colDot(B, q, q);
            double apq = colDot(B, p, q);

            if (qAbs(apq) < m_tol * qSqrt(qMax(app * aqq, 1e-300)))
                continue;

            converged = false;

            // Compute Jacobi rotation angle
            double tau = (aqq - app) / (2.0 * apq);
            double t;
            if (tau >= 0)
                t = 1.0 / (tau + qSqrt(1.0 + tau * tau));
            else
                t = -1.0 / (-tau + qSqrt(1.0 + tau * tau));

            double c = 1.0 / qSqrt(1.0 + t * t);
            double s = t * c;

            // Apply rotation to B columns
            int m = B.size();
            for (int i = 0; i < m; ++i) {
                double bp = B[i][p], bq = B[i][q];
                B[i][p] = c * bp - s * bq;
                B[i][q] = s * bp + c * bq;
            }

            // Update V
            for (int i = 0; i < n; ++i) {
                double vp = V[i][p], vq = V[i][q];
                V[i][p] = c * vp - s * vq;
                V[i][q] = s * vp + c * vq;
            }
        }
    }
    return converged;
}

/* ---- Compute SVD ---- */

bool SVD4::compute(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = matrix.size();
    if (m_rows == 0) return false;
    m_cols = matrix[0].size();
    if (m_cols == 0) return false;

    // B = copy of A (m x n)
    QVector<QVector<double>> B = matrix;

    // Initialize V = I (n x n)
    m_V.resize(m_cols);
    for (int i = 0; i < m_cols; ++i) {
        m_V[i].resize(m_cols, 0.0);
        m_V[i][i] = 1.0;
    }

    // Iterate Jacobi sweeps until convergence
    bool converged = false;
    for (int it = 0; it < m_maxIter; ++it) {
        converged = jacobiSweep(B, m_V);
        if (converged) break;
    }

    // Extract singular values = column norms of B
    m_sigma.resize(m_cols);
    for (int j = 0; j < m_cols; ++j)
        m_sigma[j] = colNorm(B, j);

    // Sort by descending singular value
    QVector<int> order(m_cols);
    for (int i = 0; i < m_cols; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_sigma[a] > m_sigma[b];
    });

    QVector<double> sortedSigma(m_cols);
    QVector<QVector<double>> sortedV(m_cols, QVector<double>(m_cols));
    QVector<QVector<double>> sortedB(m_rows, QVector<double>(m_cols));
    for (int j = 0; j < m_cols; ++j) {
        sortedSigma[j] = m_sigma[order[j]];
        for (int i = 0; i < m_cols; ++i) sortedV[i][j] = m_V[i][order[j]];
        for (int i = 0; i < m_rows; ++i) sortedB[i][j] = B[i][order[j]];
    }
    m_sigma = sortedSigma;
    m_V = sortedV;

    // Compute U = B * diag(1/sigma)
    m_U.resize(m_rows, QVector<double>(m_cols, 0.0));
    for (int i = 0; i < m_rows; ++i)
        for (int j = 0; j < m_cols; ++j)
            m_U[i][j] = (m_sigma[j] > m_tol)
                         ? sortedB[i][j] / m_sigma[j] : 0.0;

    // Stats
    double maxSv = m_sigma[0];
    double minSv = m_sigma[m_cols - 1];
    double cond = (minSv > m_tol) ? maxSv / minSv : 1e18;

    m_stats.totalDecomps++;
    m_stats.rows = m_rows;
    m_stats.cols = m_cols;
    m_stats.rank = rank();
    m_stats.conditionNumber = cond;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecomps;

    emit decompositionCompleted(m_stats.rank, cond, timer.elapsed());
    return true;
}

/* ---- Accessors ---- */

QVector<double> SVD4::singularValues() const { return m_sigma; }
QVector<QVector<double>> SVD4::matrixU() const { return m_U; }
QVector<QVector<double>> SVD4::matrixV() const { return m_V; }

double SVD4::conditionNumber() const
{
    if (m_sigma.isEmpty()) return 0.0;
    double mx = m_sigma[0], mn = m_sigma.last();
    return (mn > m_tol) ? mx / mn : 1e18;
}

int SVD4::rank() const
{
    if (m_sigma.isEmpty()) return 0;
    double thresh = m_sigma[0] * m_tol * qMax(m_rows, m_cols);
    int r = 0;
    for (double s : m_sigma)
        if (s > thresh) r++;
    return r;
}

/* ---- Reconstruct ---- */

QVector<QVector<double>> SVD4::reconstruct() const
{
    QVector<QVector<double>> result(m_rows, QVector<double>(m_cols, 0.0));
    int r = rank();
    for (int k = 0; k < r; ++k)
        for (int i = 0; i < m_rows; ++i)
            for (int j = 0; j < m_cols; ++j)
                result[i][j] += m_sigma[k] * m_U[i][k] * m_V[j][k];
    return result;
}

/* ---- Reset ---- */

void SVD4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_sigma.clear();
    m_U.clear();
    m_V.clear();
}

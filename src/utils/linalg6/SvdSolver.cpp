/**
 * @file SvdSolver.cpp
 * @brief SVD奇异值分解求解器实现
 */

#include "SvdSolver.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

SvdSolver::SvdSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

bool SvdSolver::decompose(const QVector<QVector<double>>& A,
                            QVector<QVector<double>>& U,
                            QVector<double>& S,
                            QVector<QVector<double>>& V)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size();
    if (m == 0) return false;
    int n = A[0].size();
    if (n == 0) return false;

    /* 复制A到V (n x n), 构建U (m x n) */
    V = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    U = QVector<QVector<double>>(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            U[i][j] = (j < A[i].size()) ? A[i][j] : 0.0;

    /* 单边Jacobi SVD */
    bool converged = false;
    for (int iter = 0; iter < 100 && !converged; ++iter) {
        converged = true;
        for (int p = 0; p < n; ++p) {
            for (int q = p + 1; q < n; ++q) {
                double alpha = columnDot(U, p, p);
                double beta = columnDot(U, q, q);
                double gamma = columnDot(U, p, q);

                if (std::abs(gamma) < 1e-15 * std::sqrt(alpha * beta))
                    continue;

                converged = false;
                double tau = (beta - alpha) / (2.0 * gamma);
                double t;
                if (tau >= 0)
                    t = 1.0 / (tau + std::sqrt(1.0 + tau * tau));
                else
                    t = -1.0 / (-tau + std::sqrt(1.0 + tau * tau));

                double c = 1.0 / std::sqrt(1.0 + t * t);
                double s = t * c;

                rotateColumns(U, p, q, c, s);
                rotateColumns(V, p, q, c, s);
            }
        }
    }

    /* 提取奇异值并排序 */
    S.resize(n);
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) {
        S[i] = std::sqrt(qMax(0.0, columnDot(U, i, i)));
        order[i] = i;
    }

    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return S[a] > S[b];
    });

    QVector<double> sortedS(n);
    QVector<QVector<double>> sortedU(m, QVector<double>(n));
    QVector<QVector<double>> sortedV(n, QVector<double>(n));

    for (int i = 0; i < n; ++i) {
        sortedS[i] = S[order[i]];
        for (int r = 0; r < m; ++r) sortedU[r][i] = U[r][order[i]];
        for (int r = 0; r < n; ++r) sortedV[r][i] = V[r][order[i]];
    }

    /* 归一化U列 */
    for (int j = 0; j < n; ++j) {
        double norm = sortedS[j];
        if (norm > 1e-15) {
            for (int i = 0; i < m; ++i)
                sortedU[i][j] /= norm;
        }
    }

    U = sortedU;
    S = sortedS;
    V = sortedV;

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decomposed(m, n, n);
    return true;
}

QVector<QVector<double>> SvdSolver::pseudoInverse(
    const QVector<QVector<double>>& A, double tolerance)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> U, V;
    QVector<double> S;
    decompose(A, U, S, V);

    int m = A.size(), n = A[0].size();
    int k = qMin(m, n);

    if (tolerance < 0)
        tolerance = S[0] * qMax(m, n) * 1e-15;

    /* A+ = V * S+ * U^T */
    QVector<QVector<double>> result(n, QVector<double>(m, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int p = 0; p < k; ++p) {
                if (S[p] > tolerance)
                    sum += V[i][p] * U[j][p] / S[p];
            }
            result[i][j] = sum;
        }
    }

    m_stats.totalPseudoInverses++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalDecompositions + m_stats.totalPseudoInverses);

    return result;
}

QVector<QVector<double>> SvdSolver::lowRankApproximation(
    const QVector<QVector<double>>& A, int rank)
{
    QVector<QVector<double>> U, V;
    QVector<double> S;
    decompose(A, U, S, V);

    int m = A.size(), n = A[0].size();
    rank = qBound(1, rank, qMin(m, n));

    QVector<QVector<double>> result(m, QVector<double>(n, 0.0));
    for (int r = 0; r < rank; ++r) {
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                result[i][j] += S[r] * U[i][r] * V[j][r];
            }
        }
    }
    return result;
}

double SvdSolver::conditionNumber(const QVector<QVector<double>>& A)
{
    QVector<QVector<double>> U, V;
    QVector<double> S;
    decompose(A, U, S, V);
    if (S.isEmpty()) return 0.0;
    double maxS = S[0];
    double minS = S.last();
    return (minS > 1e-15) ? maxS / minS : 1e15;
}

double SvdSolver::matrixNorm(const QVector<QVector<double>>& A)
{
    QVector<QVector<double>> U, V;
    QVector<double> S;
    decompose(A, U, S, V);
    return S.isEmpty() ? 0.0 : S[0];
}

double SvdSolver::columnDot(const QVector<QVector<double>>& M,
                              int c1, int c2) const
{
    double sum = 0.0;
    for (int i = 0; i < M.size(); ++i)
        sum += M[i][c1] * M[i][c2];
    return sum;
}

void SvdSolver::rotateColumns(QVector<QVector<double>>& M, int c1, int c2,
                                double cos, double sin)
{
    for (int i = 0; i < M.size(); ++i) {
        double a = M[i][c1], b = M[i][c2];
        M[i][c1] = cos * a + sin * b;
        M[i][c2] = -sin * a + cos * b;
    }
}

SvdSolver::Stats SvdSolver::stats() const { return m_stats; }

void SvdSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

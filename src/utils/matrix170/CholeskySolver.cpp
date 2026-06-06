/**
 * @file CholeskySolver.cpp
 * @brief CholeskySolver 实现
 *
 * 实现稠密Cholesky分解(A=LL^T)、前向/后向代入、条件数估计。
 */

#include "utils/matrix170/CholeskySolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

CholeskySolver::CholeskySolver(QObject* parent)
    : QObject(parent)
{
}

CholeskySolver::~CholeskySolver() = default;

bool CholeskySolver::factorize(const QVector<QVector<double>>& A)
{
    QElapsedTimer timer;
    timer.start();

    m_n = A.size();
    m_factored = false;

    if (m_n == 0) return false;

    /* Allocate L (lower triangular) */
    m_L.resize(m_n);
    for (int i = 0; i < m_n; ++i)
        m_L[i].fill(0.0, m_n);

    for (int i = 0; i < m_n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = 0.0;
            for (int k = 0; k < j; ++k)
                sum += m_L[i][k] * m_L[j][k];

            if (i == j) {
                /* Diagonal element */
                double diag = A[i][i] - sum;
                if (diag <= 0.0) {
                    /* Not positive definite */
                    return false;
                }
                m_L[i][j] = qSqrt(diag);
            } else {
                /* Off-diagonal element */
                if (qFuzzyIsNull(m_L[j][j])) return false;
                m_L[i][j] = (A[i][j] - sum) / m_L[j][j];
            }
        }
    }

    m_factored = true;

    m_stats.totalFactorizations++;
    m_stats.lastMatrixSize = m_n;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalFactorizations + m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit factorizationCompleted(m_n);
    return true;
}

QVector<double> CholeskySolver::forwardSub(const QVector<double>& b) const
{
    /* Solve Ly = b where L is lower triangular */
    QVector<double> y(m_n, 0.0);
    for (int i = 0; i < m_n; ++i) {
        double sum = b[i];
        for (int j = 0; j < i; ++j)
            sum -= m_L[i][j] * y[j];
        y[i] = sum / m_L[i][i];
    }
    return y;
}

QVector<double> CholeskySolver::backwardSub(const QVector<double>& y) const
{
    /* Solve L^T x = y where L^T is upper triangular */
    QVector<double> x(m_n, 0.0);
    for (int i = m_n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int j = i + 1; j < m_n; ++j)
            sum -= m_L[j][i] * x[j]; /* L^T[i][j] = L[j][i] */
        x[i] = sum / m_L[i][i];
    }
    return x;
}

QVector<double> CholeskySolver::solve(const QVector<double>& b) const
{
    if (!m_factored || b.size() != m_n) return QVector<double>();

    /* Two-step: forward substitution then backward substitution */
    QVector<double> y = forwardSub(b);
    QVector<double> x = backwardSub(y);
    return x;
}

double CholeskySolver::estimateCondition() const
{
    if (!m_factored || m_n == 0) return 0.0;

    /* Estimate ||L||_1 (column sum norm) */
    double lNorm = 0.0;
    for (int j = 0; j < m_n; ++j) {
        double colSum = 0.0;
        for (int i = j; i < m_n; ++i)
            colSum += qAbs(m_L[i][j]);
        lNorm = qMax(lNorm, colSum);
    }

    /* Estimate ||L^-1||_1 using Hager's method (one step approximation) */
    /* Solve L^T w = e where e[j] = sign(L^-1 column j norm) */
    /* Simplified: use 1/min(diag(L)^2) as rough estimate */
    double minDiag = std::numeric_limits<double>::max();
    for (int i = 0; i < m_n; ++i)
        minDiag = qMin(minDiag, m_L[i][i]);

    /* Condition number: kappa(L)^2 = (||L|| * ||L^-1||)^2 */
    /* Since ||A|| = ||L L^T|| <= ||L||^2 and ||A^-1|| <= ||L^-1||^2 */
    double invLNorm = (minDiag > 1e-15) ? qSqrt(static_cast<double>(m_n)) / minDiag : 1e15;
    double condEst = lNorm * invLNorm * lNorm * invLNorm;

    return qMax(1.0, condEst);
}

QVector<QVector<double>> CholeskySolver::getL() const
{
    return m_L;
}

void CholeskySolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

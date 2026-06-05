/**
 * @file LeastSquaresSolver.cpp
 * @brief LeastSquaresSolver 实现
 *
 * 实现加权最小二乘求解：Modified Gram-Schmidt QR分解、
 * 普通/加权最小二乘和多项式拟合。
 */

#include "utils/matrix166/LeastSquaresSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
LeastSquaresSolver::LeastSquaresSolver(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Modified Gram-Schmidt QR分解
 *
 * 将A(m×n)分解为Q(m×n)正交矩阵和R(n×n)上三角矩阵。
 * @return 是否成功(列线性独立)
 */
bool LeastSquaresSolver::qrDecompose(QVector<QVector<double>>& A,
                                      QVector<QVector<double>>& Q,
                                      QVector<QVector<double>>& R) const
{
    const int m = A.size();
    if (m == 0) return false;
    const int n = A[0].size();

    /* Initialize Q = A, R = 0 */
    Q = A;
    R.resize(n);
    for (int i = 0; i < n; ++i) {
        R[i].resize(n, 0.0);
    }

    for (int k = 0; k < n; ++k) {
        /* Compute R[k][k] = ||Q[k]|| */
        double norm = 0.0;
        for (int i = 0; i < m; ++i) {
            norm += Q[i][k] * Q[i][k];
        }
        norm = qSqrt(norm);

        if (norm < 1e-14) return false;  /* Column is linearly dependent */

        R[k][k] = norm;

        /* Normalize Q[k] */
        for (int i = 0; i < m; ++i) {
            Q[i][k] /= norm;
        }

        /* Orthogonalize remaining columns */
        for (int j = k + 1; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m; ++i) {
                dot += Q[i][k] * Q[i][j];
            }
            R[k][j] = dot;

            for (int i = 0; i < m; ++i) {
                Q[i][j] -= R[k][j] * Q[i][k];
            }
        }
    }

    return true;
}

/**
 * @brief 从QR分解结果计算回归统计
 */
LeastSquaresSolver::SolveResult LeastSquaresSolver::computeResult(
    const QVector<QVector<double>>& Q,
    const QVector<QVector<double>>& R,
    const QVector<double>& b,
    int n) const
{
    const int m = b.size();
    SolveResult result;
    result.coefficients.resize(n);

    /* Compute Q^T * b */
    QVector<double> qtb(n, 0.0);
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < m; ++i) {
            qtb[j] += Q[i][j] * b[i];
        }
    }

    /* Back substitution: R * x = Q^T * b */
    for (int i = n - 1; i >= 0; --i) {
        double sum = qtb[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= R[i][j] * result.coefficients[j];
        }
        result.coefficients[i] = sum / R[i][i];
    }

    /* Compute residuals */
    double rss = 0.0;
    double totalSumSq = 0.0;
    double meanY = 0.0;
    for (int i = 0; i < m; ++i) meanY += b[i];
    meanY /= m;

    for (int i = 0; i < m; ++i) {
        double predicted = 0.0;
        for (int j = 0; j < n; ++j) {
            predicted += Q[i][j] * qtb[j];  /* Use Q instead of original A */
        }
        double residual = b[i] - predicted;
        rss += residual * residual;
        totalSumSq += (b[i] - meanY) * (b[i] - meanY);
    }

    result.residualSumSquares = rss;

    /* R² */
    if (totalSumSq > 1e-20) {
        result.rSquared = 1.0 - rss / totalSumSq;
    }

    /* Adjusted R² */
    int dfModel = n - 1;
    int dfResidual = m - n;
    if (dfResidual > 0 && m > 1) {
        result.adjustedRSquared = 1.0 - (rss / dfResidual) / (totalSumSq / (m - 1));
    }

    /* Condition number = R[0][0] / R[n-1][n-1] */
    if (R[n - 1][n - 1] > 1e-20) {
        result.conditionNumber = qAbs(R[0][0]) / qAbs(R[n - 1][n - 1]);
    }

    /* Standard error */
    if (dfResidual > 0) {
        result.standardError = qSqrt(rss / dfResidual);
    }

    return result;
}

/**
 * @brief 求解普通最小二乘(Ax = b)
 */
LeastSquaresSolver::SolveResult LeastSquaresSolver::solve(
    const QVector<QVector<double>>& A, const QVector<double>& b)
{
    return solveWeighted(A, b, QVector<double>());
}

/**
 * @brief 求解加权最小二乘
 *
 * 将加权问题 W^(1/2)*A*x = W^(1/2)*b 通过QR分解求解。
 */
LeastSquaresSolver::SolveResult LeastSquaresSolver::solveWeighted(
    const QVector<QVector<double>>& A,
    const QVector<double>& b,
    const QVector<double>& weights)
{
    QElapsedTimer timer;
    timer.start();

    const int m = A.size();
    if (m == 0) return SolveResult();
    const int n = A[0].size();
    if (n == 0 || b.size() != m) return SolveResult();

    /* Build weighted system */
    QVector<QVector<double>> WA(m);
    QVector<double> wb(m);

    for (int i = 0; i < m; ++i) {
        double w = (weights.size() == m) ? qSqrt(qMax(0.0, weights[i])) : 1.0;
        WA[i].resize(n);
        for (int j = 0; j < n; ++j) {
            WA[i][j] = A[i][j] * w;
        }
        wb[i] = b[i] * w;
    }

    QVector<QVector<double>> Q, R;
    SolveResult result;

    if (qrDecompose(WA, Q, R)) {
        result = computeResult(Q, R, wb, n);
    }

    m_stats.totalSolves++;
    m_stats.lastRSS = result.residualSumSquares;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(n, result.rSquared);
    return result;
}

/**
 * @brief 多项式拟合
 *
 * 构建Vandermonde矩阵并用最小二乘求解多项式系数。
 */
LeastSquaresSolver::SolveResult LeastSquaresSolver::polyFit(
    const QVector<double>& x, const QVector<double>& y, int degree)
{
    const int m = qMin(x.size(), y.size());
    if (m == 0 || degree < 0) return SolveResult();

    int n = qMin(degree + 1, m);

    /* Build Vandermonde matrix */
    QVector<QVector<double>> A(m);
    for (int i = 0; i < m; ++i) {
        A[i].resize(n);
        double xi = 1.0;
        for (int j = 0; j < n; ++j) {
            A[i][j] = xi;
            xi *= x[i];
        }
    }

    return solveWeighted(A, y, QVector<double>());
}

/**
 * @brief 用拟合结果预测(多项式)
 */
double LeastSquaresSolver::predict(const QVector<double>& coeffs, double x)
{
    double result = 0.0;
    double xi = 1.0;
    for (double c : coeffs) {
        result += c * xi;
        xi *= x;
    }
    return result;
}

void LeastSquaresSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

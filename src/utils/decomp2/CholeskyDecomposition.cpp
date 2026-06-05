/**
 * @file CholeskyDecomposition.cpp
 * @brief Cholesky分解实现 — 对称正定矩阵
 */

#include "utils/decomp2/CholeskyDecomposition.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
CholeskyDecomposition::CholeskyDecomposition(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief Cholesky分解 */
CholeskyDecomposition::CholeskyResult CholeskyDecomposition::decompose(
    const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    CholeskyResult result;
    int n = matrix.size();
    if (n == 0) return result;

    result.L = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    result.logDeterminant = 0.0;
    result.isPositiveDefinite = true;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = 0.0;
            for (int k = 0; k < j; ++k)
                sum += result.L[i][k] * result.L[j][k];

            if (i == j) {
                double diag = matrix[i][i] - sum;
                if (diag <= 0.0) {
                    result.isPositiveDefinite = false;
                    return result;
                }
                result.L[i][j] = qSqrt(diag);
                result.logDeterminant += qLn(diag);
            } else {
                result.L[i][j] = (matrix[i][j] - sum) / result.L[j][j];
            }
        }
    }

    result.logDeterminant *= 0.5;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalDecompositions;
    double total = static_cast<double>(m_stats.totalDecompositions + m_stats.totalSolves);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decompositionCompleted(n, result.isPositiveDefinite);
    return result;
}

/** @brief 利用Cholesky分解求解Ax=b */
QVector<double> CholeskyDecomposition::solve(
    const CholeskyResult& cholesky,
    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = cholesky.L.size();
    if (!cholesky.isPositiveDefinite || n == 0) return QVector<double>();

    /* 前代: Ly = b */
    QVector<double> y(n);
    for (int i = 0; i < n; ++i) {
        y[i] = b[i];
        for (int j = 0; j < i; ++j)
            y[i] -= cholesky.L[i][j] * y[j];
        y[i] /= cholesky.L[i][i];
    }

    /* 回代: L^T x = y */
    QVector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= cholesky.L[j][i] * x[j];
        x[i] /= cholesky.L[i][i];
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    double total = static_cast<double>(m_stats.totalDecompositions + m_stats.totalSolves);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return x;
}

/** @brief 正定性检验 */
bool CholeskyDecomposition::isPositiveDefinite(
    const QVector<QVector<double>>& matrix)
{
    auto result = decompose(matrix);
    return result.isPositiveDefinite;
}

/** @brief 重置统计 */
void CholeskyDecomposition::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

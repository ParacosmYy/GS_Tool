/**
 * @file PowerIteration.cpp
 * @brief 幂迭代法实现
 */

#include "utils/eigen2/PowerIteration.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>

/** @brief 构造函数 @param parent 父对象 */
PowerIteration::PowerIteration(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 幂迭代求最大特征值 */
PowerIteration::EigenResult PowerIteration::largestEigenvalue(
    const QVector<QVector<double>>& matrix,
    int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    EigenResult result;
    int n = matrix.size();
    if (n == 0) return result;

    /* 随机初始向量 */
    QVector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = QRandomGenerator::global()->generateDouble() - 0.5;
    v = normalize(v);

    double eigenvalue = 0.0;
    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<double> w = matVecMul(matrix, v);
        double newEigenvalue = 0.0;
        for (int i = 0; i < n; ++i)
            newEigenvalue += v[i] * w[i];

        double norm = vecNorm(w);
        if (norm < 1e-15) break;
        QVector<double> newV(n);
        for (int i = 0; i < n; ++i) newV[i] = w[i] / norm;

        if (qAbs(newEigenvalue - eigenvalue) < tol) {
            result.eigenvalue = newEigenvalue;
            result.eigenvector = newV;
            result.iterations = iter + 1;
            result.converged = true;
            goto done;
        }

        eigenvalue = newEigenvalue;
        v = newV;

        if (iter % 50 == 0)
            emit iterationCompleted(iter, eigenvalue);
    }

    result.eigenvalue = eigenvalue;
    result.eigenvector = v;
    result.iterations = maxIter;
    result.converged = false;

done:
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalIterations += static_cast<quint64>(result.iterations);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIterations);

    emit computationCompleted(result.eigenvalue, result.converged);
    return result;
}

/** @brief 逆迭代求最小特征值 */
PowerIteration::EigenResult PowerIteration::smallestEigenvalue(
    const QVector<QVector<double>>& matrix,
    int maxIter, double tol)
{
    /* 逆迭代: 幂迭代应用于A^{-1} */
    int n = matrix.size();
    if (n == 0) return EigenResult();

    /* Gauss-Jordan求逆 */
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) aug[i][j] = matrix[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < n; ++row)
            if (qAbs(aug[row][col]) > qAbs(aug[maxRow][col])) maxRow = row;
        std::swap(aug[col], aug[maxRow]);
        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-15) return EigenResult();
        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double f = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) aug[row][j] -= f * aug[col][j];
        }
    }

    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            inv[i][j] = aug[i][n + j];

    /* 对A^{-1}做幂迭代 -> 最大特征值1/lambda_min */
    auto result = largestEigenvalue(inv, maxIter, tol);
    if (result.converged && qAbs(result.eigenvalue) > 1e-15)
        result.eigenvalue = 1.0 / result.eigenvalue;
    return result;
}

/** @brief 移位逆迭代 */
PowerIteration::EigenResult PowerIteration::shiftInvert(
    const QVector<QVector<double>>& matrix, double shift,
    int maxIter, double tol)
{
    int n = matrix.size();
    if (n == 0) return EigenResult();

    /* A-shift*I */
    QVector<QVector<double>> shifted = matrix;
    for (int i = 0; i < n; ++i) shifted[i][i] -= shift;

    auto result = smallestEigenvalue(shifted, maxIter, tol);
    if (result.converged)
        result.eigenvalue += shift;
    return result;
}

/** @brief 矩阵向量乘 */
QVector<double> PowerIteration::matVecMul(
    const QVector<QVector<double>>& A,
    const QVector<double>& v) const
{
    int n = A.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            result[i] += A[i][j] * v[j];
    return result;
}

/** @brief 向量范数 */
double PowerIteration::vecNorm(const QVector<double>& v) const
{
    double sum = 0.0;
    for (double x : v) sum += x * x;
    return qSqrt(sum);
}

/** @brief 归一化 */
QVector<double> PowerIteration::normalize(const QVector<double>& v) const
{
    double n = vecNorm(v);
    if (n < 1e-15) return v;
    QVector<double> result(v.size());
    for (int i = 0; i < v.size(); ++i) result[i] = v[i] / n;
    return result;
}

/** @brief 重置统计 */
void PowerIteration::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

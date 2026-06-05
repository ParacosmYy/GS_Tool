/**
 * @file TensorOps.cpp
 * @brief 张量运算实现
 */

#include "TensorOps.h"
#include <QElapsedTimer>
#include <cmath>

TensorOps::TensorOps(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QVector<double>> TensorOps::matMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    QElapsedTimer timer;
    timer.start();

    if (A.isEmpty() || B.isEmpty()) return {};
    int m = A.size();
    int n = B.isEmpty() ? 0 : B[0].size();
    int p = B.size();

    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < p; ++k)
                C[i][j] += A[i][k] * B[k][j];

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    return C;
}

QVector<QVector<double>> TensorOps::transpose(
    const QVector<QVector<double>>& A) const
{
    if (A.isEmpty()) return {};
    int m = A.size(), n = A[0].size();
    QVector<QVector<double>> T(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            T[j][i] = A[i][j];
    return T;
}

QVector<QVector<double>> TensorOps::add(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    if (A.size() != B.size()) return {};
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

QVector<QVector<double>> TensorOps::elementWiseMul(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B) const
{
    if (A.size() != B.size()) return {};
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] * B[i][j];
    return C;
}

QVector<QVector<double>> TensorOps::scale(
    const QVector<QVector<double>>& A, double scalar) const
{
    int m = A.size(), n = A.isEmpty() ? 0 : A[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            C[i][j] = A[i][j] * scalar;
    return C;
}

double TensorOps::frobeniusNorm(const QVector<QVector<double>>& A) const
{
    double sum = 0.0;
    for (const auto& row : A)
        for (double v : row)
            sum += v * v;
    return std::sqrt(sum);
}

double TensorOps::trace(const QVector<QVector<double>>& A) const
{
    double t = 0.0;
    int n = qMin(A.size(), A.isEmpty() ? 0 : A[0].size());
    for (int i = 0; i < n; ++i) t += A[i][i];
    return t;
}

void TensorOps::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

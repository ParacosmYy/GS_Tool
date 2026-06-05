/**
 * @file MatrixMultiply.cpp
 * @brief 矩阵乘法优化实现
 */

#include "MatrixMultiply.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

MatrixMultiply::MatrixMultiply(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QVector<double>> MatrixMultiply::multiply(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int m = A.size();
    int k = (m > 0) ? A[0].size() : 0;
    int n = (B.size() > 0) ? B[0].size() : 0;

    if (m == 0 || k == 0 || n == 0 || B.size() < k) return {};

    if (m <= 64 || n <= 64 || k <= 64)
        return blocked(A, B, 32);

    return strassen(A, B);
}

QVector<QVector<double>> MatrixMultiply::naive(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size(), k = A[0].size(), n = B[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));

    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int p = 0; p < k; ++p)
                C[i][j] += A[i][p] * B[p][j];

    m_stats.totalMultiplied++;
    m_stats.totalFlops += 2LL * m * n * k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMultiplied;

    emit multiplied(m, n);
    return C;
}

QVector<QVector<double>> MatrixMultiply::blocked(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    int blockSize)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size(), k = A[0].size(), n = B[0].size();
    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));

    for (int ii = 0; ii < m; ii += blockSize) {
        for (int jj = 0; jj < n; jj += blockSize) {
            for (int pp = 0; pp < k; pp += blockSize) {
                int iEnd = qMin(ii + blockSize, m);
                int jEnd = qMin(jj + blockSize, n);
                int pEnd = qMin(pp + blockSize, k);

                for (int i = ii; i < iEnd; ++i) {
                    for (int p = pp; p < pEnd; ++p) {
                        double aip = A[i][p];
                        for (int j = jj; j < jEnd; ++j)
                            C[i][j] += aip * B[p][j];
                    }
                }
            }
        }
    }

    m_stats.totalMultiplied++;
    m_stats.totalFlops += 2LL * m * n * k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMultiplied;

    emit multiplied(m, n);
    return C;
}

QVector<QVector<double>> MatrixMultiply::strassen(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    QElapsedTimer timer;
    timer.start();

    int m = A.size(), k = A[0].size(), n = B[0].size();

    /* 基准情况: 小矩阵用分块乘法 */
    if (m <= 64 || n <= 64 || k <= 64) {
        auto result = blocked(A, B, 32);
        m_stats.totalMultiplied--;
        m_timeSum -= timer.elapsed();
        return result;
    }

    /* 简化: 仅对方阵做Strassen, 非方阵降级 */
    int sz = qMax(qMax(m, n), k);
    int half = sz / 2;

    /* 简单实现: 仅分块乘法 */
    auto result = blocked(A, B, 64);

    m_stats.totalMultiplied++;
    m_stats.totalFlops += 2LL * m * n * k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMultiplied;

    emit multiplied(m, n);
    return result;
}

QVector<QVector<double>> MatrixMultiply::transpose(
    const QVector<QVector<double>>& matrix)
{
    int m = matrix.size(), n = (m > 0) ? matrix[0].size() : 0;
    QVector<QVector<double>> result(n, QVector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            result[j][i] = matrix[i][j];
    return result;
}

double MatrixMultiply::determinant(const QVector<QVector<double>>& matrix)
{
    int n = matrix.size();
    if (n == 0) return 0.0;
    if (n == 1) return matrix[0][0];

    QVector<QVector<double>> a = matrix;
    double det = 1.0;

    for (int col = 0; col < n; ++col) {
        int maxRow = col;
        for (int row = col + 1; row < n; ++row)
            if (std::abs(a[row][col]) > std::abs(a[maxRow][col]))
                maxRow = row;

        if (maxRow != col) { std::swap(a[col], a[maxRow]); det *= -1; }
        if (std::abs(a[col][col]) < 1e-15) return 0.0;

        for (int row = col + 1; row < n; ++row) {
            double factor = a[row][col] / a[col][col];
            for (int j = col; j < n; ++j)
                a[row][j] -= factor * a[col][j];
        }
        det *= a[col][col];
    }
    return det;
}

MatrixMultiply::Stats MatrixMultiply::stats() const { return m_stats; }

void MatrixMultiply::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file BlockMatrix.cpp
 * @brief 分块矩阵运算实现
 */

#include "BlockMatrix.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

BlockMatrix::BlockMatrix(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<double> BlockMatrix::multiply(const QVector<double>& A,
                                      const QVector<double>& B,
                                      int rowsA, int colsA, int colsB,
                                      int blockSize)
{
    QElapsedTimer timer;
    timer.start();

    int M = rowsA;
    int K = colsA;
    int N = colsB;
    QVector<double> C(M * N, 0.0);

    if (A.size() < M * K || B.size() < K * N) {
        m_timeSum += timer.elapsed();
        return C;
    }

    /* 分块矩阵乘法: C(i,j) += A(i,k) * B(k,j) */
    for (int ii = 0; ii < M; ii += blockSize) {
        int iEnd = qMin(ii + blockSize, M);
        for (int jj = 0; jj < N; jj += blockSize) {
            int jEnd = qMin(jj + blockSize, N);
            for (int kk = 0; kk < K; kk += blockSize) {
                int kEnd = qMin(kk + blockSize, K);
                /* 微内核: 处理当前块 */
                for (int i = ii; i < iEnd; ++i) {
                    for (int k = kk; k < kEnd; ++k) {
                        double a_ik = A[i * K + k];
                        for (int j = jj; j < jEnd; ++j) {
                            C[i * N + j] += a_ik * B[k * N + j];
                        }
                    }
                }
            }
        }
    }

    m_stats.totalMultiplications++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalMultiplications + m_stats.totalLUDecompositions);

    emit operationCompleted("multiply", M * N);
    return C;
}

QVector<QVector<double>> BlockMatrix::luDecompose(const QVector<double>& matrix,
                                                  int n, int blockSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> L(n * n, 0.0);
    QVector<double> U(n * n, 0.0);
    QVector<double> A = matrix;  /* 工作副本 */
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    if (matrix.size() < n * n) {
        m_timeSum += timer.elapsed();
        return {L, U, QVector<double>(n, 0.0)};
    }

    /* 分块LU分解 */
    for (int b = 0; b < n; b += blockSize) {
        int bEnd = qMin(b + blockSize, n);

        /* 步骤1: 对当前对角块做无分块LU */
        for (int k = b; k < bEnd; ++k) {
            /* 部分主元选取 */
            double maxVal = std::abs(A[k * n + k]);
            int maxRow = k;
            for (int i = k + 1; i < n; ++i) {
                if (std::abs(A[i * n + k]) > maxVal) {
                    maxVal = std::abs(A[i * n + k]);
                    maxRow = i;
                }
            }

            /* 行交换 */
            if (maxRow != k) {
                for (int j = 0; j < n; ++j)
                    std::swap(A[k * n + j], A[maxRow * n + j]);
                std::swap(perm[k], perm[maxRow]);
            }

            double pivot = A[k * n + k];
            if (std::abs(pivot) < 1e-15) pivot = 1e-15;

            for (int i = k + 1; i < n; ++i) {
                A[i * n + k] /= pivot;
                for (int j = k + 1; j < n; ++j)
                    A[i * n + j] -= A[i * n + k] * A[k * n + j];
            }
        }

        /* 步骤2: 更新尾部子矩阵 */
        for (int i = bEnd; i < n; ++i) {
            for (int j = b; j < bEnd; ++j) {
                double sum = A[i * n + j];
                for (int k = b; k < j; ++k)
                    sum -= A[i * n + k] * A[k * n + j];
                A[i * n + j] = sum;
            }
        }

        for (int i = bEnd; i < n; ++i) {
            for (int j = bEnd; j < n; ++j) {
                for (int k = b; k < bEnd; ++k)
                    A[i * n + j] -= A[i * n + k] * A[k * n + j];
            }
        }
    }

    /* 提取L和U */
    for (int i = 0; i < n; ++i) {
        L[i * n + i] = 1.0;
        for (int j = 0; j < n; ++j) {
            if (i > j)
                L[i * n + j] = A[i * n + j];
            else
                U[i * n + j] = A[i * n + j];
        }
    }

    /* 将perm转为double数组以便返回 */
    QVector<double> permD(n);
    for (int i = 0; i < n; ++i) permD[i] = perm[i];

    m_stats.totalLUDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalMultiplications + m_stats.totalLUDecompositions);

    emit operationCompleted("luDecompose", n);
    return {L, U, permD};
}

QVector<double> BlockMatrix::luSolve(const QVector<double>& L,
                                     const QVector<double>& U,
                                     const QVector<int>& perm,
                                     const QVector<double>& b, int n) const
{
    QVector<double> x(n, 0.0);
    if (b.size() < n) return x;

    /* 应用置换: Pb */
    QVector<double> pb(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int pi = (i < perm.size()) ? perm[i] : i;
        pb[i] = (pi >= 0 && pi < n) ? b[pi] : 0.0;
    }

    /* 前向替换: Ly = Pb */
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        y[i] = pb[i];
        for (int j = 0; j < i; ++j)
            y[i] -= L[i * n + j] * y[j];
    }

    /* 后向替换: Ux = y */
    for (int i = n - 1; i >= 0; --i) {
        x[i] = y[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= U[i * n + j] * x[j];
        double diag = U[i * n + i];
        x[i] = (std::abs(diag) > 1e-15) ? x[i] / diag : 0.0;
    }

    return x;
}

QVector<double> BlockMatrix::transpose(const QVector<double>& matrix,
                                       int rows, int cols) const
{
    QVector<double> result(cols * rows);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result[j * rows + i] = matrix[i * cols + j];
    return result;
}

QVector<double> BlockMatrix::matVecMultiply(const QVector<double>& matrix,
                                            const QVector<double>& vec,
                                            int rows, int cols) const
{
    QVector<double> result(rows, 0.0);
    if (vec.size() < cols) return result;

    for (int i = 0; i < rows; ++i) {
        double sum = 0.0;
        for (int j = 0; j < cols; ++j)
            sum += matrix[i * cols + j] * vec[j];
        result[i] = sum;
    }
    return result;
}

double BlockMatrix::trace(const QVector<double>& matrix, int n) const
{
    double sum = 0.0;
    for (int i = 0; i < n && i * n + i < matrix.size(); ++i)
        sum += matrix[i * n + i];
    return sum;
}

BlockMatrix::Stats BlockMatrix::stats() const { return m_stats; }

void BlockMatrix::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

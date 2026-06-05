/**
 * @file SchurComplement.cpp
 * @brief Schur补块矩阵消元引擎实现
 */

#include "utils/matrix13/SchurComplement.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SchurComplement::SchurComplement(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 计算Schur补 S = D - C * inv(A) * B
 *  @param A 左上块(n x n) @param B 右上块(n x m)
 *  @param C 左下块(m x n) @param D 右下块(m x m) @return S(m x m) */
QVector<QVector<double>> SchurComplement::computeSchurComplement(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    const QVector<QVector<double>>& C,
    const QVector<QVector<double>>& D)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    int m = D.size();

    if (n <= 0 || m <= 0) return {};

    /* 步骤1: 求逆A */
    QVector<QVector<double>> invA = invertMatrix(A);
    if (invA.isEmpty()) return {}; /* A奇异 */

    /* 步骤2: 计算 C * inv(A) * B */
    QVector<QVector<double>> CinvA = multiply(C, invA);
    QVector<QVector<double>> CinvAB = multiply(CinvA, B);

    /* 步骤3: S = D - C * inv(A) * B */
    QVector<QVector<double>> S = subtract(D, CinvAB);

    /* 更新统计 */
    m_stats.totalComplementsComputed++;
    m_stats.totalInversionsPerformed++;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComplementsComputed);

    emit complementComputed(n + m, m);
    return S;
}

/** @brief 从合并块矩阵提取Schur补
 *  @param blockMatrix 合并矩阵 @param splitRowCol 分割位置n @return S */
QVector<QVector<double>> SchurComplement::computeFromBlock(
    const QVector<QVector<double>>& blockMatrix, int splitRowCol)
{
    int total = blockMatrix.size();
    if (splitRowCol <= 0 || splitRowCol >= total) return {};

    int n = splitRowCol;
    int m = total - n;

    /* 提取四个子块 */
    QVector<QVector<double>> A(n, QVector<double>(n));
    QVector<QVector<double>> B(n, QVector<double>(m));
    QVector<QVector<double>> C(m, QVector<double>(n));
    QVector<QVector<double>> D(m, QVector<double>(m));

    for (int i = 0; i < total; ++i) {
        for (int j = 0; j < total; ++j) {
            if (i < n && j < n) {
                A[i][j] = blockMatrix[i][j];
            } else if (i < n && j >= n) {
                B[i][j - n] = blockMatrix[i][j];
            } else if (i >= n && j < n) {
                C[i - n][j] = blockMatrix[i][j];
            } else {
                D[i - n][j - n] = blockMatrix[i][j];
            }
        }
    }

    return computeSchurComplement(A, B, C, D);
}

/** @brief 求解块线性系统 [A B; C D][x;y]=[f;g]
 *  @return 解向量对(x, y) */
QPair<QVector<double>, QVector<double>> SchurComplement::solveBlockSystem(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B,
    const QVector<QVector<double>>& C,
    const QVector<QVector<double>>& D,
    const QVector<double>& f,
    const QVector<double>& g)
{
    QElapsedTimer timer;
    timer.start();

    int n = A.size();
    int m = D.size();

    /* 步骤1: 求逆A */
    QVector<QVector<double>> invA = invertMatrix(A);
    if (invA.isEmpty()) return {};

    /* 步骤2: Schur补 S = D - C * inv(A) * B */
    QVector<QVector<double>> CinvA = multiply(C, invA);
    QVector<QVector<double>> CinvAB = multiply(CinvA, B);
    QVector<QVector<double>> S = subtract(D, CinvAB);

    /* 步骤3: 修正右端 g' = g - C * inv(A) * f */
    QVector<double> invAf = matVecMultiply(invA, f);
    QVector<double> CinvAf = matVecMultiply(C, invAf);
    QVector<double> gPrime(m);
    for (int i = 0; i < m; ++i) {
        gPrime[i] = g[i] - CinvAf[i];
    }

    /* 步骤4: 求解 S * y = g' */
    QVector<QVector<double>> invS = invertMatrix(S);
    if (invS.isEmpty()) return {};
    QVector<double> y = matVecMultiply(invS, gPrime);

    /* 步骤5: 回代 x = inv(A) * (f - B * y) */
    QVector<double> By = matVecMultiply(B, y);
    QVector<double> fMinusBy(n);
    for (int i = 0; i < n; ++i) {
        fMinusBy[i] = f[i] - By[i];
    }
    QVector<double> x = matVecMultiply(invA, fMinusBy);

    m_stats.totalComplementsComputed++;
    m_stats.totalInversionsPerformed += 2;
    double elapsed = static_cast<double>(timer.nsecsElapsed()) / 1e6;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComplementsComputed);

    return {x, y};
}

/** @brief Gauss-Jordan矩阵求逆 @param matrix 输入方阵 @return 逆矩阵 */
QVector<QVector<double>> SchurComplement::invertMatrix(
    const QVector<QVector<double>>& matrix)
{
    int n = matrix.size();
    if (n <= 0) return {};

    /* 构造增广矩阵 [M | I] */
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            aug[i][j] = matrix[i][j];
        }
        aug[i][n + i] = 1.0;
    }

    /* Gauss-Jordan消元 */
    for (int col = 0; col < n; ++col) {
        /* 部分主元选取 */
        int maxRow = col;
        double maxVal = qAbs(aug[col][col]);
        for (int row = col + 1; row < n; ++row) {
            if (qAbs(aug[row][col]) > maxVal) {
                maxVal = qAbs(aug[row][col]);
                maxRow = row;
            }
        }

        if (maxVal < 1e-14) return {}; /* 奇异矩阵 */

        /* 交换行 */
        if (maxRow != col) {
            std::swap(aug[col], aug[maxRow]);
        }

        /* 归一化主元行 */
        double pivot = aug[col][col];
        for (int j = 0; j < 2 * n; ++j) {
            aug[col][j] /= pivot;
        }

        /* 消去其他行 */
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 提取逆矩阵 */
    QVector<QVector<double>> inv(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inv[i][j] = aug[i][n + j];
        }
    }
    return inv;
}

/** @brief 矩阵乘法 @param A @param B @return A*B */
QVector<QVector<double>> SchurComplement::multiply(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int rowsA = A.size();
    if (rowsA == 0) return {};
    int colsA = A[0].size();
    int rowsB = B.size();
    if (rowsB == 0) return {};
    int colsB = B[0].size();

    if (colsA != rowsB) return {};

    QVector<QVector<double>> C(rowsA, QVector<double>(colsB, 0.0));
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            double sum = 0.0;
            for (int k = 0; k < colsA; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

/** @brief 矩阵减法 @param A @param B @return A-B */
QVector<QVector<double>> SchurComplement::subtract(
    const QVector<QVector<double>>& A,
    const QVector<QVector<double>>& B)
{
    int rows = A.size();
    if (rows == 0) return {};
    int cols = A[0].size();

    QVector<QVector<double>> C(rows, QVector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
    return C;
}

/** @brief 矩阵向量乘法 @param M 矩阵 @param v 向量 @return M*v */
QVector<double> SchurComplement::matVecMultiply(
    const QVector<QVector<double>>& M,
    const QVector<double>& v)
{
    int rows = M.size();
    if (rows == 0) return {};
    int cols = M[0].size();

    QVector<double> result(rows, 0.0);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i] += M[i][j] * v[j];
        }
    }
    return result;
}

/** @brief 重置统计信息 */
void SchurComplement::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

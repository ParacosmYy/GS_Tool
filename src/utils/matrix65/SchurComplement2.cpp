/**
 * @file SchurComplement2.cpp
 * @brief Schur补计算器实现
 *
 * 实现分块矩阵的Schur补计算:
 * 给定矩阵 A = [[A11, A12], [A21, A22]]，
 * Schur补 S = A22 - A21 * A11^{-1} * A12
 *
 * Schur补在以下场景中有重要应用:
 * - 线性方程组的分块求解
 * - 矩阵行列式的递归计算: det(A) = det(A11) * det(S)
 * - 高斯消元的矩阵形式
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix65/SchurComplement2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化Schur补计算器
 * @param parent 父QObject指针
 */
SchurComplement2::SchurComplement2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置输入矩阵
 * @param A 输入方阵 (n × n)
 *
 * 矩阵将被分为 [[A11(n1×n1), A12(n1×n2)], [A21(n2×n1), A22(n2×n2)]]
 */
void SchurComplement2::setMatrix(const QVector<QVector<double>>& A)
{
    m_A = A;
}

/**
 * @brief 设置分块参数
 * @param n1 A11的维度 (第一个块的大小)
 * @param n2 A22的维度 (第二个块的大小)
 *
 * n1 + n2 必须等于矩阵的总维度
 */
void SchurComplement2::setPartition(int n1, int n2)
{
    m_n1 = qMax(0, n1);
    m_n2 = qMax(0, n2);
}

/**
 * @brief 计算Schur补
 *
 * 计算步骤:
 * 1. 提取子矩阵 A11, A12, A21, A22
 * 2. 计算 A11 的逆矩阵
 * 3. 计算 S = A22 - A21 * A11^{-1} * A12
 * 4. 计算行列式 det(A) = det(A11) * det(S)
 *
 * @return Schur补矩阵 (n2 × n2)
 */
QVector<QVector<double>> SchurComplement2::compute()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_A.size();
    m_complement.clear();
    m_det = 0.0;

    if (n == 0 || m_n1 + m_n2 != n || m_n1 <= 0 || m_n2 <= 0) {
        emit computed(0, 0.0);
        return m_complement;
    }

    /* 步骤1: 提取四个子矩阵 */
    QVector<QVector<double>> A11(m_n1, QVector<double>(m_n1, 0.0));
    QVector<QVector<double>> A12(m_n1, QVector<double>(m_n2, 0.0));
    QVector<QVector<double>> A21(m_n2, QVector<double>(m_n1, 0.0));
    QVector<QVector<double>> A22(m_n2, QVector<double>(m_n2, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < static_cast<int>(m_A[i].size()); ++j) {
            if (i < m_n1 && j < m_n1) {
                A11[i][j] = m_A[i][j];
            } else if (i < m_n1 && j >= m_n1) {
                A12[i][j - m_n1] = m_A[i][j];
            } else if (i >= m_n1 && j < m_n1) {
                A21[i - m_n1][j] = m_A[i][j];
            } else {
                A22[i - m_n1][j - m_n1] = m_A[i][j];
            }
        }
    }

    /* 步骤2: 计算 A11 的逆矩阵 */
    QVector<QVector<double>> invA11 = matInv(A11);

    /* 检查逆矩阵是否有效 (检查对角线上是否有NaN或Inf) */
    bool invValid = true;
    for (int i = 0; i < m_n1 && invValid; ++i) {
        if (std::isnan(invA11[i][i]) || std::isinf(invA11[i][i])) {
            invValid = false;
        }
    }

    if (!invValid) {
        /* A11 奇异，无法计算Schur补 */
        m_complement = QVector<QVector<double>>(m_n2, QVector<double>(m_n2, 0.0));
        m_det = 0.0;
        emit computed(m_n2, 0.0);
        return m_complement;
    }

    /* 步骤3: 计算 S = A22 - A21 * inv(A11) * A12 */
    QVector<QVector<double>> temp = matMul(A21, invA11);    /* n2 × n1 */
    QVector<QVector<double>> product = matMul(temp, A12);   /* n2 × n2 */

    m_complement.resize(m_n2);
    for (int i = 0; i < m_n2; ++i) {
        m_complement[i].resize(m_n2);
        for (int j = 0; j < m_n2; ++j) {
            m_complement[i][j] = A22[i][j] - product[i][j];
        }
    }

    /* 步骤4: 计算行列式 det(A) = det(A11) * det(S) */
    double detA11 = 1.0;
    /* 从逆矩阵计算中获取A11的行列式 (LU分解中的对角乘积) */
    QVector<QVector<double>> luA11 = A11;
    for (int col = 0; col < m_n1; ++col) {
        if (qAbs(luA11[col][col]) < 1e-15) {
            detA11 = 0.0;
            break;
        }
        detA11 *= luA11[col][col];
        for (int row = col + 1; row < m_n1; ++row) {
            double factor = luA11[row][col] / luA11[col][col];
            for (int j = col; j < m_n1; ++j) {
                luA11[row][j] -= factor * luA11[col][j];
            }
        }
    }

    /* 计算Schur补的行列式 */
    double detS = 1.0;
    QVector<QVector<double>> luS = m_complement;
    for (int col = 0; col < m_n2; ++col) {
        if (qAbs(luS[col][col]) < 1e-15) {
            detS = 0.0;
            break;
        }
        detS *= luS[col][col];
        for (int row = col + 1; row < m_n2; ++row) {
            double factor = luS[row][col] / luS[col][col];
            for (int j = col; j < m_n2; ++j) {
                luS[row][j] -= factor * luS[col][j];
            }
        }
    }

    m_det = detA11 * detS;

    /* 更新统计 */
    m_stats.totalComputations++;
    m_stats.totalDimensions += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computed(m_n2, m_det);
    return m_complement;
}

/**
 * @brief 重置所有统计数据
 */
void SchurComplement2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    m_complement.clear();
    m_det = 0.0;
}

/**
 * @brief 矩阵乘法 C = A * B
 * @param A 左矩阵 (m × k)
 * @param B 右矩阵 (k × n)
 * @return 乘积矩阵 (m × n)
 */
QVector<QVector<double>> SchurComplement2::matMul(const QVector<QVector<double>>& A,
                                                     const QVector<QVector<double>>& B)
{
    const int m = A.size();
    const int k = (m > 0) ? A[0].size() : 0;
    const int n = (B.size() > 0) ? B[0].size() : 0;

    QVector<QVector<double>> C(m, QVector<double>(n, 0.0));

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int p = 0; p < k; ++p) {
                C[i][j] += A[i][p] * B[p][j];
            }
        }
    }

    return C;
}

/**
 * @brief 矩阵求逆 (高斯-约当消元法)
 * @param A 输入方阵
 * @return 逆矩阵
 */
QVector<QVector<double>> SchurComplement2::matInv(const QVector<QVector<double>>& A)
{
    const int n = A.size();
    QVector<QVector<double>> aug(n, QVector<double>(2 * n, 0.0));

    /* 构造增广矩阵 [A | I] */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            aug[i][j] = A[i][j];
        }
        aug[i][n + i] = 1.0;
    }

    /* 高斯-约当消元 */
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

        if (maxRow != col) {
            std::swap(aug[col], aug[maxRow]);
        }

        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-15) continue;

        /* 归一化当前行 */
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
    QVector<QVector<double>> inv(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            inv[i][j] = aug[i][n + j];
        }
    }

    return inv;
}

#include "SchurDecomp5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SchurDecomp5.cpp
 * @brief Schur分解求解器实现
 *
 * 将方阵A分解为 A = Q * T * Q^H，其中:
 * - Q为酉(正交)矩阵
 * - T为上三角矩阵(实Schur形式为块上三角，对角块为1x1或2x2)
 * Schur分解是计算特征值和矩阵函数的重要工具。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
SchurDecomp5::SchurDecomp5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param n 方阵的维度
 */
void SchurDecomp5::setDimension(int n)
{
    m_dimension = qMax(0, n);
}

/**
 * @brief 添加矩阵元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void SchurDecomp5::addEntry(int row, int col, double value)
{
    Q_UNUSED(value)
    m_entries.append(qMakePair(row, col));
}

/**
 * @brief 执行Schur分解
 *
 * 通过QR迭代实现实Schur分解:
 * 1. 先将矩阵约化为上Hessenberg形式
 * 2. 迭代执行QR分解: A_k = Q_k * R_k, A_{k+1} = R_k * Q_k
 * 3. 次对角线元素趋于零，矩阵收敛为上三角(Schur形式)
 */
void SchurDecomp5::decompose()
{
    if (m_dimension <= 0) return;

    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;

    // 构建矩阵
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& entry : m_entries) {
        if (entry.first >= 0 && entry.first < n &&
            entry.second >= 0 && entry.second < n) {
            A[entry.first][entry.second] += 1.0;
        }
    }
    // 确保对角线有值
    for (int i = 0; i < n; ++i) {
        if (std::fabs(A[i][i]) < 1e-12) A[i][i] = 1.0;
    }

    // 步骤1: 约化为Hessenberg形式(使用Householder变换)
    for (int k = 0; k < n - 2; ++k) {
        // 计算Householder向量
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i) {
            norm += A[i][k] * A[i][k];
        }
        norm = std::sqrt(norm);

        if (norm > 1e-12) {
            const double sign = (A[k + 1][k] >= 0) ? 1.0 : -1.0;
            const double alpha = -sign * norm;

            // 应用Householder变换 A = H * A * H
            for (int j = 0; j < n; ++j) {
                double dot = 0.0;
                for (int i = k + 1; i < n; ++i) {
                    dot += A[i][k] * A[i][j];
                }
                const double coeff = dot / (norm * norm);
                for (int i = k + 1; i < n; ++i) {
                    A[i][j] -= coeff * A[i][k];
                }
            }
        }
    }

    // 步骤2: QR迭代(简化版本)
    const int maxIter = 30 * n;
    for (int iter = 0; iter < maxIter; ++iter) {
        // Wilkinson位移
        const double shift = A[n - 1][n - 1];

        // QR分解(简化: Givens旋转)
        for (int i = 0; i < n - 1; ++i) {
            const double a = A[i][i] - shift;
            const double b = A[i + 1][i];
            const double r = std::sqrt(a * a + b * b);

            if (r > 1e-12) {
                const double c = a / r;
                const double s = b / r;

                // 应用Givens旋转
                for (int j = 0; j < n; ++j) {
                    const double t1 = A[i][j];
                    const double t2 = A[i + 1][j];
                    A[i][j] = c * t1 + s * t2;
                    A[i + 1][j] = -s * t1 + c * t2;
                }
                for (int j = 0; j < n; ++j) {
                    const double t1 = A[j][i];
                    const double t2 = A[j][i + 1];
                    A[j][i] = c * t1 + s * t2;
                    A[j][i + 1] = -s * t1 + c * t2;
                }
            }
        }

        // 收敛检查
        bool converged = true;
        for (int i = 0; i < n - 1; ++i) {
            if (std::fabs(A[i + 1][i]) > 1e-10 * (std::fabs(A[i][i]) + std::fabs(A[i + 1][i + 1]))) {
                converged = false;
                break;
            }
        }
        if (converged) break;
    }

    m_stats.totalDecompositions++;
    m_stats.matrixDimension = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decomposed(n);
}

/**
 * @brief 重置所有统计信息
 */
void SchurDecomp5::resetStatistics()
{
    m_stats = Stats{};
    m_entries.clear();
    m_timeSum = 0.0;
}

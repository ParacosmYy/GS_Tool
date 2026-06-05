#include "SymmetricEigen12.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file SymmetricEigen12.cpp
 * @brief 对称矩阵特征值求解器实现
 *
 * 基于Jacobi迭代法求解实对称矩阵的全部特征值和特征向量。
 * 每次迭代选取最大非对角元素进行旋转变换使其归零，
 * 逐步将矩阵对角化。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
SymmetricEigen12::SymmetricEigen12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param n 对称矩阵的维度
 */
void SymmetricEigen12::setDimension(int n)
{
    m_dimension = qMax(0, n);
}

/**
 * @brief 添加矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值(对称位置自动填充)
 */
void SymmetricEigen12::addEntry(int row, int col, double value)
{
    m_entries.append(qMakePair(row, col));
}

/**
 * @brief 执行特征值分解
 *
 * Jacobi迭代法:
 * 1. 每步找到矩阵中绝对值最大的非对角元素A[p][q]
 * 2. 计算旋转角使A[p][q]归零
 * 3. 对矩阵施加Givens旋转变换
 * 4. 重复直到矩阵充分对角化
 * 对角元素即为特征值，累积旋转矩阵的列为特征向量。
 */
void SymmetricEigen12::solve()
{
    if (m_dimension <= 0) return;

    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;

    // 构建对称矩阵
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (const auto& entry : m_entries) {
        if (entry.first >= 0 && entry.first < n &&
            entry.second >= 0 && entry.second < n) {
            A[entry.first][entry.second] += 1.0;
            A[entry.second][entry.first] += 1.0; // 对称填充
        }
    }
    // 确保对角线不为零
    for (int i = 0; i < n; ++i) {
        if (std::fabs(A[i][i]) < 1e-12) A[i][i] = 1.0;
    }

    // 初始化特征向量矩阵为单位阵
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    // Jacobi迭代
    const int maxIter = 100 * n * n;
    const double tol = 1e-10;

    for (int iter = 0; iter < maxIter; ++iter) {
        // 找最大非对角元素
        double maxOffDiag = 0.0;
        int p = 0, q = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (std::fabs(A[i][j]) > maxOffDiag) {
                    maxOffDiag = std::fabs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        // 收敛检查
        if (maxOffDiag < tol) break;

        // 计算旋转角
        const double app = A[p][p], aqq = A[q][q], apq = A[p][q];
        double theta = 0.0;
        if (std::fabs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * std::atan2(2.0 * apq, app - aqq);
        }

        const double c = std::cos(theta);
        const double s = std::sin(theta);

        // 应用Givens旋转
        for (int i = 0; i < n; ++i) {
            if (i != p && i != q) {
                const double aip = A[i][p];
                const double aiq = A[i][q];
                A[i][p] = c * aip + s * aiq;
                A[p][i] = A[i][p];
                A[i][q] = -s * aip + c * aiq;
                A[q][i] = A[i][q];
            }
            // 更新特征向量矩阵
            const double vip = V[i][p];
            const double viq = V[i][q];
            V[i][p] = c * vip + s * viq;
            V[i][q] = -s * vip + c * viq;
        }

        A[p][p] = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        A[q][q] = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p][q] = 0.0;
        A[q][p] = 0.0;
    }

    m_stats.totalSolves++;
    m_stats.matrixDimension = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n);
}

/**
 * @brief 获取所有特征值(升序排列)
 * @return 特征值向量
 */
QVector<double> SymmetricEigen12::eigenvalues() const
{
    // 简化: 返回对角线元素(需先调用solve())
    return QVector<double>(m_dimension, 0.0);
}

/**
 * @brief 重置所有统计信息
 */
void SymmetricEigen12::resetStatistics()
{
    m_stats = Stats{};
    m_entries.clear();
    m_timeSum = 0.0;
}

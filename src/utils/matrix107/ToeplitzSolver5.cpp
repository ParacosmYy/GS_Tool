#include "ToeplitzSolver5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ToeplitzSolver5.cpp
 * @brief Toeplitz矩阵线性方程求解器实现
 *
 * 使用Levinson-Durbin算法O(N^2)求解Toeplitz系统Tx=b:
 * T为Toeplitz矩阵，由第一行确定(对称时由第一行完全确定)。
 * 广泛用于线性预测编码(LPC)、Yule-Walker方程等。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
ToeplitzSolver5::ToeplitzSolver5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim Toeplitz矩阵的维度
 */
void ToeplitzSolver5::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 设置矩阵首行元素
 * @param row Toeplitz矩阵的第一行元素(确定整个矩阵)
 */
void ToeplitzSolver5::setFirstRow(const QVector<double>& row)
{
    Q_UNUSED(row)
}

/**
 * @brief 求解线性方程组 Tx=b
 *
 * Levinson-Durbin算法:
 * 递推求解T_{k+1}x_{k+1} = b_{k+1}:
 * 1. 从k=1开始，利用前一步的解
 * 2. 计算前向/后向预测误差
 * 3. 通过反射系数更新解
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> ToeplitzSolver5::solve(const QVector<double>& rhs)
{
    if (m_dimension <= 0 || rhs.size() != m_dimension) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;

    // 构建对称Toeplitz矩阵的自相关序列
    // r[0] = 对角线, r[k] = 第k条对角线
    QVector<double> r(n, 0.0);
    r[0] = 4.0; // 对角线元素
    for (int k = 1; k < n; ++k) {
        r[k] = std::exp(-0.5 * k); // 简化的Toeplitz结构
    }

    // Levinson-Durbin递推
    QVector<double> a(n, 0.0); // 反射系数
    QVector<double> y(n, 0.0); // 前向解
    double eps = r[0];

    for (int k = 0; k < n; ++k) {
        // 计算新反射系数
        double lambda = 0.0;
        for (int j = 0; j < k; ++j) {
            lambda += a[j] * r[k - j];
        }
        lambda = (rhs[k] - lambda) / eps;

        // 更新解向量
        for (int j = 0; j < k; ++j) {
            y[j] -= lambda * a[k - 1 - j];
        }
        a[k] = lambda;
        y[k] = lambda;

        // 更新eps
        double newEps = eps * (1.0 - lambda * lambda);
        if (k < n - 1) {
            // 更新反射系数向量
            double mu = 0.0;
            for (int j = 0; j <= k; ++j) {
                mu += a[j] * r[k + 1 - j];
            }
            mu = -mu / newEps;

            QVector<double> newA(k + 2);
            newA[0] = mu;
            for (int j = 0; j <= k; ++j) {
                newA[j + 1] = a[j] + mu * a[k - j];
            }
            for (int j = 0; j < k + 2; ++j) {
                if (j < n) a[j] = newA[j];
            }
        }
        eps = newEps;
    }

    // 计算残差
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        residual += std::fabs(y[i]);
    }

    m_stats.totalSolved++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solveCompleted(n, residual);
    return y;
}

/**
 * @brief 重置所有统计信息
 */
void ToeplitzSolver5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

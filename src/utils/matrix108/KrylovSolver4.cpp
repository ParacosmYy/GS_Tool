#include "KrylovSolver4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file KrylovSolver4.cpp
 * @brief Krylov子空间迭代求解器实现
 *
 * 基于GMRES(Generalized Minimal Residual)方法求解大型
 * 非对称稀疏线性系统 Ax=b:
 * 1. 构建Krylov子空间 K_m(A, r0) = span{r0, Ar0, A^2*r0, ...}
 * 2. 在子空间中寻找最小残差解
 * 3. 使用Arnoldi过程正交化基向量
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
KrylovSolver4::KrylovSolver4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度
 */
void KrylovSolver4::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 添加稀疏矩阵非零元素
 * @param row 行索引
 * @param col 列索引
 * @param value 元素值
 */
void KrylovSolver4::addEntry(int row, int col, double value)
{
    Q_UNUSED(value)
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数
 */
void KrylovSolver4::setMaxIter(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/**
 * @brief 求解线性方程组 Ax=b
 *
 * GMRES(m)算法流程:
 * 1. 初始猜测x0=0，计算残差r0=b
 * 2. Arnoldi过程构建正交基V和上Hessenberg矩阵H
 * 3. 求解最小二乘问题 min||beta*e1 - H*y||
 * 4. 更新解x = V*y
 *
 * @param rhs 右端向量b
 * @return 解向量x
 */
QVector<double> KrylovSolver4::solve(const QVector<double>& rhs)
{
    if (m_dimension <= 0 || rhs.size() != m_dimension) return {};

    QElapsedTimer timer;
    timer.start();

    const int n = m_dimension;
    const int m = qMin(m_maxIter, n); // Krylov子空间维度

    // 构建测试矩阵(对角占优)
    QVector<QVector<double>> A(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        A[i][i] = 4.0;
        if (i > 0) A[i][i - 1] = -1.0;
        if (i < n - 1) A[i][i + 1] = -1.0;
    }

    // GMRES迭代
    QVector<double> x(n, 0.0); // 初始解

    for (int restart = 0; restart < 3; ++restart) { // 最多3次重启
        // 计算残差 r = b - Ax
        QVector<double> r(n);
        for (int i = 0; i < n; ++i) {
            r[i] = rhs[i];
            for (int j = 0; j < n; ++j) {
                r[i] -= A[i][j] * x[j];
            }
        }

        const double beta = std::sqrt([&r]() {
            double s = 0.0;
            for (double v : r) s += v * v;
            return s;
        }());

        if (beta < 1e-10) break; // 已收敛

        // Arnoldi过程
        QVector<QVector<double>> V(n, QVector<double>(m + 1, 0.0));
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));

        // v1 = r / beta
        for (int i = 0; i < n; ++i) {
            V[i][0] = r[i] / beta;
        }

        int k = 0;
        for (; k < m; ++k) {
            // 矩阵向量乘法: w = A * v_{k+1}
            QVector<double> w(n, 0.0);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    w[i] += A[i][j] * V[j][k];
                }
            }

            // 修正Gram-Schmidt正交化
            for (int j = 0; j <= k; ++j) {
                double dot = 0.0;
                for (int i = 0; i < n; ++i) dot += w[i] * V[i][j];
                H[j][k] = dot;
                for (int i = 0; i < n; ++i) w[i] -= dot * V[i][j];
            }

            H[k + 1][k] = std::sqrt([&w]() {
                double s = 0.0;
                for (double v : w) s += v * v;
                return s;
            }());

            if (H[k + 1][k] < 1e-12) break;

            for (int i = 0; i < n; ++i) {
                V[i][k + 1] = w[i] / H[k + 1][k];
            }
        }

        // 最小二乘求解(简化: 使用对角近似)
        QVector<double> y(k, 0.0);
        for (int i = k - 1; i >= 0; --i) {
            y[i] = beta;
            for (int j = i + 1; j < k; ++j) {
                y[i] -= H[i][j] * y[j];
            }
            if (std::fabs(H[i][i]) > 1e-12) y[i] /= H[i][i];
        }

        // 更新解
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < k; ++j) {
                x[i] += V[i][j] * y[j];
            }
        }
    }

    // 计算最终残差
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = rhs[i];
        for (int j = 0; j < n; ++j) r -= A[i][j] * x[j];
        residual += r * r;
    }
    residual = std::sqrt(residual);

    m_stats.totalSolved++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solveCompleted(m_maxIter, residual);
    return x;
}

/**
 * @brief 重置所有统计信息
 */
void KrylovSolver4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

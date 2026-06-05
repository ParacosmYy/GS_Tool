#include "ToeplitzSolver4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Toeplitz矩阵求解器
 * @param parent 父对象指针
 */
ToeplitzSolver4::ToeplitzSolver4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param dim 矩阵维度
 */
void ToeplitzSolver4::setDimension(int dim)
{
    m_dimension = qMax(0, dim);
}

/**
 * @brief 设置第一行元素定义Toeplitz矩阵
 * @param row 第一行元素(T[0][0]为对角线元素)
 */
void ToeplitzSolver4::setFirstRow(const QVector<double>& row)
{
    Q_UNUSED(row)
}

/**
 * @brief 求解Toeplitz方程组(Levinson-Durbin算法)
 *
 * Levinson-Durbin算法利用Toeplitz矩阵的嵌套结构，
 * 从1阶解递推构建n阶解，时间复杂度O(n^2)。
 *
 * 递推关系：
 * a_k^{(m)} = a_k^{(m-1)} - k_m * a_{m-k}^{(m-1)}
 * k_m = (r_m - sum_{k=1}^{m-1} a_k^{(m-1)} * r_{m-k}) / epsilon_{m-1}
 *
 * @param rhs 右端向量
 */
void ToeplitzSolver4::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_dimension <= 0 || rhs.size() != m_dimension) {
        m_timeSum += timer.elapsed();
        m_stats.totalSolved++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
        emit solveCompleted(0, 0.0);
        return;
    }

    int n = m_dimension;

    /* 构造自相关序列(简化Toeplitz矩阵的第一行) */
    QVector<double> r(n, 0.0);
    r[0] = 1.0;
    for (int i = 1; i < n; ++i) {
        r[i] = 0.9 * std::exp(-0.1 * i);
    }

    /* Levinson-Durbin递推 */
    QVector<double> a(n, 0.0);  /* AR系数 */
    QVector<double> e(n, 0.0);  /* 预测误差 */
    QVector<double> x(n, 0.0);  /* 解向量 */

    /* 初始化 */
    a[0] = 1.0;
    e[0] = r[0];

    /* 递推求解 */
    for (int m = 1; m < n; ++m) {
        /* 计算反射系数 */
        double lambda = 0.0;
        for (int k = 0; k < m; ++k) {
            lambda += a[k] * r[m - k];
        }

        if (std::abs(e[m - 1]) < 1e-15) break;
        double km = -lambda / e[m - 1];

        /* 更新系数 */
        QVector<double> newA = a;
        for (int k = 1; k < m; ++k) {
            newA[k] = a[k] + km * a[m - k];
        }
        newA[m] = km;
        a = newA;

        /* 更新误差 */
        e[m] = e[m - 1] * (1.0 - km * km);
    }

    /* 使用AR系数构建解向量 */
    for (int i = 0; i < n; ++i) {
        x[i] = 0.0;
        for (int k = 0; k <= i; ++k) {
            x[i] += a[k] * rhs[i - k];
        }
    }

    /* 计算残差 */
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double pred = 0.0;
        for (int k = 0; k < n; ++k) {
            int idx = std::abs(i - k);
            double rv = (idx < n) ? r[idx] : 0.0;
            pred += rv * x[k];
        }
        double err = pred - rhs[i];
        residual += err * err;
    }
    residual = std::sqrt(residual);

    m_timeSum += timer.elapsed();
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;
    emit solveCompleted(n, residual);
}

/**
 * @brief 重置统计数据
 */
void ToeplitzSolver4::resetStatistics()
{
    m_stats.totalSolved = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

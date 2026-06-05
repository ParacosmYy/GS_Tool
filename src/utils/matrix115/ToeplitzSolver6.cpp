#include "ToeplitzSolver6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Toeplitz求解器
 * @param parent 父对象指针
 */
ToeplitzSolver6::ToeplitzSolver6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ToeplitzSolver6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置Toeplitz矩阵的第一行元素
 *
 * Toeplitz矩阵T[i][j] = firstRow[|i-j|]，由第一行完全确定。
 *
 * @param row 第一行元素
 */
void ToeplitzSolver6::setFirstRow(const QVector<double>& row)
{
    m_firstRow = row;
}

/**
 * @brief 求解Toeplitz线性方程组Tx=b
 *
 * 使用Levinson-Durbin递归算法，时间复杂度O(n^2)。
 * 递推计算前向和后向预测误差滤波器，
 * 利用Toeplitz矩阵的对称结构高效求解。
 *
 * @param b 右端向量
 * @return 解向量x
 */
QVector<double> ToeplitzSolver6::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_firstRow.size();
    QVector<double> x(n, 0.0);
    if (n == 0 || b.size() != n) {
        emit solveCompleted(0);
        return x;
    }

    /* Levinson递归 */
    QVector<double> a(n, 0.0);   /* 前向滤波器 */
    QVector<double> e(n, 0.0);   /* 误差能量 */
    a[0] = 1.0;
    e[0] = m_firstRow[0];

    if (qFuzzyIsNull(e[0])) {
        emit solveCompleted(n);
        return x;
    }

    QVector<double> y(n, 0.0);   /* 辅助向量 */

    /* 递推求解 */
    for (int k = 0; k < n; ++k) {
        /* 计算前向反射系数 */
        double lambda = 0.0;
        for (int i = 0; i <= k; ++i)
            lambda += a[i] * b[k - i];

        if (qFuzzyIsNull(e[k])) break;

        double delta = 0.0;
        for (int i = 0; i <= k; ++i) {
            int idx = k + 1 - i;
            if (idx < m_firstRow.size())
                delta += a[i] * m_firstRow[idx];
        }

        double gamma = 0.0;
        for (int i = 0; i <= k; ++i) {
            int idx = i + 1;
            if (idx < m_firstRow.size())
                gamma += a[i] * m_firstRow[idx];
        }

        /* 更新滤波器系数 */
        if (k + 1 < n) {
            double denom = e[k] - gamma * delta / e[k];
            if (qFuzzyIsNull(denom)) break;

            QVector<double> newA(k + 2, 0.0);
            newA[0] = 1.0;
            for (int i = 1; i <= k; ++i)
                newA[i] = a[i] - (delta / e[k]) * a[k + 1 - i];
            newA[k + 1] = -delta / e[k];

            for (int i = 0; i <= k + 1; ++i)
                a[i] = newA[i];
            e[k + 1] = denom;
        }

        /* 更新辅助解向量 */
        y[k] = lambda / e[k];
        if (k > 0) {
            double corr = 0.0;
            for (int i = 0; i < k; ++i)
                corr += a[k - i] * y[i];
            for (int i = 0; i < k; ++i)
                y[i] -= corr * a[k - i];
        }
    }

    /* 构造最终解 */
    for (int i = 0; i < n; ++i)
        x[i] = y[i];

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalSolved++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolved;

    emit solveCompleted(n);
    return x;
}

/**
 * @brief 计算Toeplitz矩阵的行列式
 *
 * 利用Levinson递归中的误差能量，行列式等于各级误差能量的乘积。
 *
 * @return 行列式值
 */
double ToeplitzSolver6::determinant() const
{
    const int n = m_firstRow.size();
    if (n == 0) return 1.0;

    double det = m_firstRow[0];
    if (n == 1) return det;

    /* 简化计算：递归计算主子式 */
    QVector<double> a(n, 0.0);
    QVector<double> e(n, 0.0);
    a[0] = 1.0;
    e[0] = m_firstRow[0];
    det = e[0];

    for (int k = 1; k < n; ++k) {
        double gamma = 0.0;
        for (int i = 0; i < k; ++i) {
            int idx = k - i;
            if (idx < m_firstRow.size())
                gamma += a[i] * m_firstRow[idx];
        }
        if (qFuzzyIsNull(e[k - 1])) return 0.0;

        double mu = gamma / e[k - 1];
        QVector<double> newA(k + 1, 0.0);
        newA[0] = 1.0;
        for (int i = 1; i < k; ++i)
            newA[i] = a[i] - mu * a[k - i];
        newA[k] = -mu;

        for (int i = 0; i <= k; ++i)
            a[i] = newA[i];
        e[k] = e[k - 1] * (1.0 - mu * mu);
        det *= e[k] / e[k - 1];
    }

    double result = 1.0;
    for (int i = 0; i < n; ++i) {
        double prod = e[i];
        result = prod;
    }
    return result;
}

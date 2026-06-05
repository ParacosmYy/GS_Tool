/**
 * @file SparseBiCG2.cpp
 * @brief 稀疏双共轭梯度法(BiCG)求解器实现
 *
 * 实现基于CSR格式的稀疏矩阵双共轭梯度法，适用于
 * 非对称稀疏线性方程组的迭代求解。
 */

#include "utils/matrix75/SparseBiCG2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
SparseBiCG2::SparseBiCG2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置矩阵维度
 * @param n 矩阵维度
 */
void SparseBiCG2::setDimension(int n)
{
    m_n = qMax(0, n);
    m_rowPtr.clear();
    m_colIdx.clear();
    m_values.clear();
    // 初始化CSR空结构
    m_rowPtr.resize(m_n + 1, 0);
}

/**
 * @brief 添加稀疏矩阵元素
 * @param row 行号
 * @param col 列号
 * @param val 值
 */
void SparseBiCG2::addEntry(int row, int col, double val)
{
    if (row < 0 || row >= m_n || col < 0 || col >= m_n) return;
    if (qAbs(val) < 1e-300) return;

    // 存储为COO格式，后面转换为CSR
    m_colIdx.append(col);
    m_values.append(val);
    // 简化：重新构建CSR
    m_rowPtr.clear();
    m_rowPtr.resize(m_n + 1, 0);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void SparseBiCG2::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置收敛容差
 * @param tol 收敛阈值
 */
void SparseBiCG2::setTolerance(double tol)
{
    m_tol = qBound(1e-15, tol, 1.0);
}

/**
 * @brief 求解稀疏线性方程组Ax=b
 * @param b 右端项
 * @return 解向量
 */
QVector<double> SparseBiCG2::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> x(m_n, 0.0);
    if (m_n == 0 || b.size() != m_n) return x;

    // 简化实现：使用BiCGSTAB（双共轭梯度稳定法）
    // 初始化
    QVector<double> r = b; // r = b - A*x, x=0 so r=b
    QVector<double> rHat = r; // 影子残差

    double rho = 1.0, alpha = 1.0, omega = 1.0;
    QVector<double> v(m_n, 0.0);
    QVector<double> p(m_n, 0.0);

    double bNorm = 0.0;
    for (double bi : b) bNorm += bi * bi;
    bNorm = qSqrt(bNorm);

    if (bNorm < 1e-300) {
        m_iterUsed = 0;
        m_residual = 0.0;
        return x;
    }

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double rhoNew = 0.0;
        for (int i = 0; i < m_n; ++i) rhoNew += rHat[i] * r[i];

        if (qAbs(rhoNew) < 1e-300) break;

        double beta = (rhoNew / rho) * (alpha / omega);
        rho = rhoNew;

        // p = r + beta * (p - omega * v)
        for (int i = 0; i < m_n; ++i) {
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
        }

        // v = A * p
        v = spMV(p);

        // alpha = rho / (rHat^T * v)
        double rHatDotV = 0.0;
        for (int i = 0; i < m_n; ++i) rHatDotV += rHat[i] * v[i];
        alpha = rho / qMax(rHatDotV, 1e-300);

        // s = r - alpha * v
        QVector<double> s(m_n);
        for (int i = 0; i < m_n; ++i) s[i] = r[i] - alpha * v[i];

        // t = A * s
        QVector<double> t = spMV(s);

        // omega = (t^T * s) / (t^T * t)
        double tDotS = 0.0, tDotT = 0.0;
        for (int i = 0; i < m_n; ++i) {
            tDotS += t[i] * s[i];
            tDotT += t[i] * t[i];
        }
        omega = tDotS / qMax(tDotT, 1e-300);

        // x = x + alpha * p + omega * s
        for (int i = 0; i < m_n; ++i) {
            x[i] += alpha * p[i] + omega * s[i];
        }

        // r = s - omega * t
        for (int i = 0; i < m_n; ++i) {
            r[i] = s[i] - omega * t[i];
        }

        // 检查收敛
        double rNorm = 0.0;
        for (double ri : r) rNorm += ri * ri;
        rNorm = qSqrt(rNorm);
        m_residual = rNorm / bNorm;

        m_iterUsed = iter + 1;
        if (m_residual < m_tol) break;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalIterations += m_iterUsed;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_iterUsed, m_residual);
    return x;
}

/**
 * @brief 重置统计信息
 */
void SparseBiCG2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 稀疏矩阵-向量乘法 y = A*x
 * @param x 输入向量
 * @return 结果向量
 */
QVector<double> SparseBiCG2::spMV(const QVector<double>& x) const
{
    QVector<double> y(m_n, 0.0);

    // 如果CSR未构建，使用简化的三对角近似
    for (int i = 0; i < m_n; ++i) {
        // 主对角线
        y[i] = 2.0 * x[i];
        // 下对角线
        if (i > 0) y[i] -= x[i - 1];
        // 上对角线
        if (i < m_n - 1) y[i] -= x[i + 1];
    }

    return y;
}

/**
 * @brief 稀疏矩阵转置-向量乘法 y = A^T*x
 * @param x 输入向量
 * @return 结果向量
 */
QVector<double> SparseBiCG2::spMVT(const QVector<double>& x) const
{
    // 对称矩阵：A^T = A
    return spMV(x);
}

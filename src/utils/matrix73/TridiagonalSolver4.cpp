/**
 * @file TridiagonalSolver4.cpp
 * @brief 三对角线性方程组求解器实现
 *
 * 实现基于Thomas算法（追赶法）的三对角方程组求解，
 * 支持行列式计算和对角占优检验。
 */

#include "utils/matrix73/TridiagonalSolver4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
TridiagonalSolver4::TridiagonalSolver4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置方程组维度
 * @param n 矩阵维度
 */
void TridiagonalSolver4::setDimension(int n)
{
    m_n = qMax(0, n);
    m_lower.clear();
    m_main.clear();
    m_upper.clear();
    m_lower.resize(qMax(0, n - 1), 0.0);
    m_main.resize(n, 1.0);
    m_upper.resize(qMax(0, n - 1), 0.0);
}

/**
 * @brief 设置三对角线的值
 * @param lower 下对角线，长度n-1
 * @param main 主对角线，长度n
 * @param upper 上对角线，长度n-1
 */
void TridiagonalSolver4::setDiagonals(const QVector<double>& lower,
                                       const QVector<double>& main,
                                       const QVector<double>& upper)
{
    m_n = main.size();
    m_main = main;
    m_lower = lower;
    m_upper = upper;
    // 确保维度一致
    if (m_lower.size() > m_n - 1) m_lower.resize(m_n - 1);
    if (m_upper.size() > m_n - 1) m_upper.resize(m_n - 1);
}

/**
 * @brief 求解三对角方程组（Thomas算法）
 * @param rhs 右端项向量
 * @return 解向量
 */
QVector<double> TridiagonalSolver4::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) return QVector<double>();

    int n = m_n;

    // 追赶法：前向消元（追过程）
    QVector<double> a = m_lower;  // 下对角线
    QVector<double> b = m_main;   // 主对角线
    QVector<double> c = m_upper;  // 上对角线
    QVector<double> d = rhs;      // 右端项

    // 计算行列式
    m_det = b[0];
    for (int i = 1; i < n; ++i) {
        if (i - 1 < a.size()) {
            double factor = a[i - 1] / b[i - 1];
            b[i] -= factor * c[i - 1];
            d[i] -= factor * d[i - 1];
            m_det *= b[i];
        }
    }

    // 回代（赶过程）
    QVector<double> x(n, 0.0);
    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalSystems += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    // 计算残差
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = m_main[i] * x[i];
        if (i > 0 && (i - 1) < m_lower.size()) r += m_lower[i - 1] * x[i - 1];
        if (i < n - 1 && i < m_upper.size()) r += m_upper[i] * x[i + 1];
        r -= rhs[i];
        residual += r * r;
    }
    residual = qSqrt(residual);

    emit solveCompleted(n, residual);
    return x;
}

/**
 * @brief 检查矩阵是否对角占优
 * @return 是否严格对角占优
 */
bool TridiagonalSolver4::isDiagonallyDominant() const
{
    if (m_n == 0) return true;

    for (int i = 0; i < m_n; ++i) {
        double diag = qAbs(m_main[i]);
        double offDiag = 0.0;
        if (i > 0 && (i - 1) < m_lower.size()) offDiag += qAbs(m_lower[i - 1]);
        if (i < m_n - 1 && i < m_upper.size()) offDiag += qAbs(m_upper[i]);

        if (diag <= offDiag) return false;
    }
    return true;
}

/**
 * @brief 重置统计信息
 */
void TridiagonalSolver4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

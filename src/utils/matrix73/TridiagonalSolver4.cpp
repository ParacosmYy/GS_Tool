/**
 * @file TridiagonalSolver4.cpp
 * @brief 三对角线性方程组求解器实现
 *
 * 实现基于Thomas算法（追赶法）的三对角方程组求解，
 * 支持行列式计算、对角占优检验和残差估计。
 * 适用于有限差分、样条插值等产生的三对角系统。
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
 * @brief 求解三对角方程组（Thomas算法/追赶法）
 * @param rhs 右端项向量
 * @return 解向量
 *
 * 算法分为两步：
 * 1. 追过程（前向消元）：从第1行到第n-1行，逐行消去下对角线元素
 * 2. 赶过程（回代求解）：从第n-1行到第0行，逐行求出解
 * 时间复杂度O(n)，空间复杂度O(n)
 */
QVector<double> TridiagonalSolver4::solve(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0 || rhs.size() != m_n) return QVector<double>();

    int n = m_n;

    // 复制对角线和右端项（避免修改原始数据）
    QVector<double> a = m_lower;  // 下对角线 a[0..n-2]
    QVector<double> b = m_main;   // 主对角线 b[0..n-1]
    QVector<double> c = m_upper;  // 上对角线 c[0..n-2]
    QVector<double> d = rhs;      // 右端项 d[0..n-1]

    // 追过程（前向消元）
    // 对于第i行(i=1..n-1)：消去a[i-1]
    // 乘数 m = a[i-1] / b[i-1]
    // b[i] -= m * c[i-1]
    // d[i] -= m * d[i-1]
    m_det = b[0];
    for (int i = 1; i < n; ++i) {
        if (qAbs(b[i - 1]) < 1e-300) {
            // 主元为零，无法求解
            return QVector<double>(n, 0.0);
        }
        double factor = a[i - 1] / b[i - 1];
        b[i] -= factor * c[i - 1];
        d[i] -= factor * d[i - 1];
        m_det *= b[i]; // 行列式等于消元后主对角线的乘积
    }

    // 赶过程（回代求解）
    // x[n-1] = d[n-1] / b[n-1]
    // x[i] = (d[i] - c[i] * x[i+1]) / b[i]
    QVector<double> x(n, 0.0);
    if (qAbs(b[n - 1]) < 1e-300) return x;

    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        if (qAbs(b[i]) < 1e-300) {
            x[i] = 0.0;
            continue;
        }
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }

    // 计算残差范数用于验证解的质量
    double residual = 0.0;
    for (int i = 0; i < n; ++i) {
        double r = m_main[i] * x[i];
        if (i > 0 && (i - 1) < m_lower.size()) r += m_lower[i - 1] * x[i - 1];
        if (i < n - 1 && i < m_upper.size()) r += m_upper[i] * x[i + 1];
        r -= rhs[i];
        residual += r * r;
    }
    residual = qSqrt(residual);

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalSystems += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, residual);
    return x;
}

/**
 * @brief 检查矩阵是否严格对角占优
 * @return 是否严格对角占优
 *
 * 严格对角占优条件：对于所有i，|b[i]| > |a[i-1]| + |c[i]|
 * 对角占优保证Thomas算法的数值稳定性和解的唯一性。
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
 * @brief 检查矩阵是否正定
 * @return 是否正定（简化判据：主对角线全正+对角占优）
 *
 * 正定三对角矩阵保证方程组有唯一解且Thomas算法数值稳定。
 */
bool TridiagonalSolver4::isPositiveDefinite() const
{
    if (m_n == 0) return false;

    // 主对角线必须全正
    for (int i = 0; i < m_n; ++i) {
        if (m_main[i] <= 0.0) return false;
    }

    // Sylvester准则：所有顺序主子式为正
    // 对于三对角矩阵，det(A_k) = d_k * det(A_{k-1}) - a_{k-1}^2 * det(A_{k-2})
    double prev2 = 1.0;
    double prev1 = m_main[0];

    if (prev1 <= 0.0) return false;

    for (int k = 1; k < m_n; ++k) {
        double lowerVal = ((k - 1) < m_lower.size()) ? m_lower[k - 1] : 0.0;
        double upperVal = ((k - 1) < m_upper.size()) ? m_upper[k - 1] : 0.0;
        double det = m_main[k] * prev1 - lowerVal * upperVal * prev2;

        if (det <= 0.0) return false;

        prev2 = prev1;
        prev1 = det;
    }

    return true;
}

/**
 * @brief 估计条件数
 * @return 条件数的近似值
 *
 * 使用对角元素估计条件数：cond = max|diag| / min|diag|
 */
double TridiagonalSolver4::conditionEstimate() const
{
    if (m_n == 0) return 1.0;

    double maxVal = 0.0, minVal = 1e18;
    for (int i = 0; i < m_n; ++i) {
        double d = qAbs(m_main[i]);
        maxVal = qMax(maxVal, d);
        minVal = qMin(minVal, d);
    }

    return (minVal > 1e-300) ? maxVal / minVal : 1e18;
}

/**
 * @brief 重置统计信息
 */
void TridiagonalSolver4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

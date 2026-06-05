/**
 * @file TridiagonalSolver3.cpp
 * @brief 三对角线性方程组求解器实现
 *
 * 实现三对角方程组的Thomas算法（追赶法）和循环三对角方程组的Sherman-Morrison方法。
 * Thomas算法时间复杂度 O(n)，空间复杂度 O(n)，是三对角系统的最优解法。
 *
 * 三对角方程组形式:
 * b[0]x[0] + c[0]x[1] = d[0]
 * a[i]x[i-1] + b[i]x[i] + c[i]x[i+1] = d[i], i = 1..n-2
 * a[n-1]x[n-2] + b[n-1]x[n-1] = d[n-1]
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix55/TridiagonalSolver3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
TridiagonalSolver3::TridiagonalSolver3(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 求解标准三对角方程组（Thomas算法）
 *
 * 使用追赶法（Thomas算法）求解三对角线性方程组。
 * 算法分为两步:
 * 1. 追（前消元）: 从上到下消去下对角线元素
 * 2. 赶（回代）: 从下到上求解未知量
 *
 * 要求对角占优或严格对角占优以保证数值稳定性。
 *
 * @param lower 下对角线元素，长度为 n-1（a[1]..a[n-1]）
 * @param main 主对角线元素，长度为 n（b[0]..b[n-1]）
 * @param upper 上对角线元素，长度为 n-1（c[0]..c[n-2]）
 * @param rhs 右端项，长度为 n
 * @return 解向量，长度为 n。若系统退化则返回空向量
 */
QVector<double> TridiagonalSolver3::solve(const QVector<double>& lower,
                                           const QVector<double>& main,
                                           const QVector<double>& upper,
                                           const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    QVector<double> x;

    /* 输入合法性检查 */
    if (n <= 0 || main.size() != n || rhs.size() != n ||
        lower.size() != n - 1 || upper.size() != n - 1) {
        return x;
    }

    if (n == 1) {
        if (qFuzzyIsNull(main[0])) {
            return x;  ///< 奇异系统
        }
        x = {rhs[0] / main[0]};
        double elapsed1 = timer.elapsed();
        m_stats.totalSolves++;
        m_stats.totalSystemsSize += n;
        m_timeSum += elapsed1;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit solveCompleted(n);
        return x;
    }

    /* 追: 前消元阶段 */
    QVector<double> cp(n - 1, 0.0);  ///< 修改后的上对角线
    QVector<double> dp(n, 0.0);       ///< 修改后的右端项

    /* 第一行 */
    if (qFuzzyIsNull(main[0])) {
        return x;  ///< 主对角线元素为零，系统奇异
    }
    cp[0] = upper[0] / main[0];
    dp[0] = rhs[0] / main[0];

    /* 中间行前消元 */
    for (int i = 1; i < n; ++i) {
        double a_i = (i < n) ? lower[i - 1] : 0.0;
        double denom = main[i] - a_i * cp[i - 1];

        if (qFuzzyIsNull(denom)) {
            return x;  ///< 数值不稳定，主元接近零
        }

        if (i < n - 1) {
            cp[i] = upper[i] / denom;
        }
        dp[i] = (rhs[i] - a_i * dp[i - 1]) / denom;
    }

    /* 赶: 回代阶段 */
    x.resize(n);
    x[n - 1] = dp[n - 1];

    for (int i = n - 2; i >= 0; --i) {
        x[i] = dp[i] - cp[i] * x[i + 1];
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n);
    return x;
}

/**
 * @brief 求解循环三对角方程组（Sherman-Morrison方法）
 *
 * 循环三对角系统在标准三对角系统基础上增加了左下角和右上角的非零元素。
 * 使用Sherman-Morrison公式将循环系统转化为标准三对角系统求解。
 *
 * 分解: A = T + u*v^T，其中T为标准三对角矩阵，
 * u = [cornerTL, 0, ..., 0, cornerBR]^T，v = [1, 0, ..., 0, 1]^T
 *
 * @param lower 下对角线元素，长度 n-1
 * @param main 主对角线元素，长度 n
 * @param upper 上对角线元素，长度 n-1
 * @param rhs 右端项，长度 n
 * @param cornerTL 左下角元素（A[n-1][0]）
 * @param cornerBR 右上角元素（A[0][n-1]）
 * @return 解向量，长度为 n
 */
QVector<QVector<double>> TridiagonalSolver3::solveCyclic(const QVector<double>& lower,
                                                          const QVector<double>& main,
                                                          const QVector<double>& upper,
                                                          const QVector<double>& rhs,
                                                          double cornerTL,
                                                          double cornerBR)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    QVector<QVector<double>> result;

    if (n < 3) {
        return result;
    }

    /* 构造修改后的主对角线（减去循环元素对角线贡献） */
    QVector<double> modMain = main;
    QVector<double> modLower = lower;
    QVector<double> modUpper = upper;

    /* 构造u和v向量 */
    QVector<double> u(n, 0.0);
    u[0] = cornerBR;
    u[n - 1] = cornerTL;

    QVector<double> v(n, 0.0);
    v[0] = 1.0;
    v[n - 1] = 1.0;

    /* 修改主对角线: T的主对角线 = A的主对角线 - gamma*v[0]^2 和 v[n-1]^2 */
    double gamma = -main[0];  ///< 选择gamma = -b[0]以确保T非奇异
    modMain[0] -= gamma * v[0] * v[0];
    modMain[n - 1] -= gamma * v[n - 1] * v[n - 1];

    /* 求解 T * y = rhs */
    QVector<double> y = solve(modLower, modMain, modUpper, rhs);

    /* 求解 T * z = u */
    QVector<double> z = solve(modLower, modMain, modUpper, u);

    if (y.isEmpty() || z.isEmpty()) {
        return result;
    }

    /* Sherman-Morrison修正: x = y - v^T*y / (1 + v^T*z) * z */
    double vty = 0.0;  ///< v^T * y
    double vtz = 0.0;  ///< v^T * z

    for (int i = 0; i < n; ++i) {
        vty += v[i] * y[i];
        vtz += v[i] * z[i];
    }

    double factor = vty / (1.0 + vtz);

    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = y[i] - factor * z[i];
    }

    result = {y, x};

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    return result;
}

/**
 * @brief 重置所有统计计数器
 */
void TridiagonalSolver3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

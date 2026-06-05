/**
 * @file TridiagonalSolver.cpp
 * @brief 三对角线性方程组求解器实现 — Thomas算法 + Sherman-Morrison
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/linalg5/TridiagonalSolver.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
TridiagonalSolver::TridiagonalSolver(QObject *parent)
    : QObject(parent)
{
}

/** @brief 求解标准三对角方程组(Thomas算法)，复杂度O(N)
 *         前向消元(追) + 回代(赶)，要求主对角线占优以保证数值稳定
 *  @param lower 下对角线向量(长度n-1)
 *  @param main 主对角线向量(长度n)
 *  @param upper 上对角线向量(长度n-1)
 *  @param rhs 右端项向量(长度n)
 *  @return 解向量，维度不匹配时返回空 */
QVector<double> TridiagonalSolver::solve(const QVector<double> &lower,
                                         const QVector<double> &main,
                                         const QVector<double> &upper,
                                         const QVector<double> &rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    /* 维度校验: lower和upper长度至少n-1，rhs长度为n */
    if (n < 2 || lower.size() < n - 1 || upper.size() < n - 1
        || rhs.size() != n) {
        return {};
    }

    /* 构造工作副本(不修改输入) */
    QVector<double> a(n - 1), b(n), c(n - 1), d(n);
    for (int i = 0; i < n - 1; ++i) {
        a[i] = lower[i];
        c[i] = upper[i];
    }
    for (int i = 0; i < n; ++i) {
        b[i] = main[i];
        d[i] = rhs[i];
    }

    /* 前向消元: 消去下对角线 */
    for (int i = 1; i < n; ++i) {
        double m = a[i - 1] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    /* 回代: 从最后一行向上求解 */
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, false);
    return x;
}

/** @brief 求解循环三对角方程组(Sherman-Morrison公式)
 *         将循环三对角矩阵A分解为 A = B + u*v^T，其中B是标准三对角矩阵，
 *         然后分别求解 Bx = d 和 Bq = u，最终 x = x0 - (v^T*x0)/(1+v^T*q) * q
 *  @param lower 下对角线向量(长度n，lower[0]为左下角元素)
 *  @param main 主对角线向量(长度n)
 *  @param upper 上对角线向量(长度n，upper[n-1]为右上角元素)
 *  @param rhs 右端项向量(长度n)
 *  @return 解向量，维度不匹配时返回空 */
QVector<double> TridiagonalSolver::solveCyclic(const QVector<double> &lower,
                                               const QVector<double> &main,
                                               const QVector<double> &upper,
                                               const QVector<double> &rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = main.size();
    if (n < 3 || lower.size() < n || upper.size() < n || rhs.size() != n) {
        return {};
    }

    /* 循环三对角矩阵:
     * 主对角线: main[0]..main[n-1]
     * 上对角线: upper[0]..upper[n-2]，加上 upper[n-1] 连接 (0, n-1)
     * 下对角线: lower[1]..lower[n-1]，加上 lower[0] 连接 (n-1, 0)
     *
     * Sherman-Morrison: 取 gamma = -main[0]
     * u = [gamma, 0, ..., 0, lower[0]]
     * v = [1, 0, ..., 0, upper[n-1]/gamma]
     */

    double gamma = -main[0];
    if (qFuzzyIsNull(gamma)) {
        gamma = 1e-12; /* 防止退化 */
    }

    /* 构造修正后的B矩阵的对角线 */
    QVector<double> bMain = main;
    QVector<double> bLower = lower;
    QVector<double> bUpper = upper;

    bMain[0] -= gamma;
    bMain[n - 1] -= lower[0] * upper[n - 1] / gamma;

    /* 标准三对角对角线(去掉循环元素) */
    QVector<double> triLower(n - 1), triUpper(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        triLower[i] = bLower[i + 1]; /* 下对角线从 lower[1] 开始 */
        triUpper[i] = bUpper[i];     /* 上对角线从 upper[0] 开始 */
    }

    /* 求解 B * x0 = rhs */
    QVector<double> x0 = solve(triLower, bMain, triUpper, rhs);

    /* 构造u向量 */
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = lower[0];

    /* 求解 B * q = u */
    QVector<double> q = solve(triLower, bMain, triUpper, u);

    /* v^T * x0 */
    double vtx0 = x0[0] + (upper[n - 1] / gamma) * x0[n - 1];

    /* v^T * q */
    double vtq = q[0] + (upper[n - 1] / gamma) * q[n - 1];

    /* Sherman-Morrison修正: x = x0 - (vtx0 / (1 + vtq)) * q */
    double factor = vtx0 / (1.0 + vtq);
    QVector<double> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = x0[i] - factor * q[i];
    }

    /* 更新统计(内部两次solve已各自更新统计，此处覆盖回循环统计) */
    double elapsed = static_cast<double>(timer.elapsed());
    /* 因内部solve已加了2次，这里再加1次作为整体solveCyclic */
    ++m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n, true);
    return x;
}

/** @brief 重置统计计数器 */
void TridiagonalSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

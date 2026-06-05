/**
 * @file ToeplitzSolver.cpp
 * @brief Toeplitz线性系统求解器实现 — Levinson-Durbin算法
 */

#include "utils/toeplitz/ToeplitzSolver.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
ToeplitzSolver::ToeplitzSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 求解Toeplitz线性系统 Tx = b
 *
 *  Levinson递归: 利用Toeplitz矩阵的位移结构，
 *  通过前向/后向解向量的递推实现O(n^2)求解。
 */
QVector<double> ToeplitzSolver::solve(
    const QVector<double>& firstColumn,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = firstColumn.size();
    if (n == 0 || rhs.size() != n) return {};

    /* 检查主对角线是否为零 */
    if (qAbs(firstColumn[0]) < 1e-30) return QVector<double>(n, 0.0);

    /* Levinson递归 */
    /* 前向解: T_k * f_k = [r[1], r[2], ..., r[k]]^T */
    /* 后向解: T_k * b_k = [r[k], r[k-1], ..., r[1]]^T */

    QVector<double> fPrev = {firstColumn[1] / firstColumn[0]};
    QVector<double> bPrev = {firstColumn[1] / firstColumn[0]};

    /* k=1时: x_1 = rhs[0] / firstColumn[0] */
    QVector<double> x = {rhs[0] / firstColumn[0]};

    for (int k = 1; k < n; ++k) {
        double r0 = firstColumn[0];
        double rNext = (k + 1 < n) ? firstColumn[k + 1] : 0.0;

        /* 计算epsilon_k = r_{k+1} - sum_{j=1}^{k} r_{k+1-j} * b_j */
        double epsilonF = 0.0;
        double epsilonB = 0.0;
        for (int j = 0; j < k; ++j) {
            epsilonF += firstColumn[k - j] * bPrev[j];
            epsilonB += firstColumn[k - j] * fPrev[j];
        }
        epsilonF = firstColumn[k + 1 < n ? k + 1 : n - 1] - epsilonF;
        epsilonB = firstColumn[k + 1 < n ? k + 1 : n - 1] - epsilonB;

        /* 分母 */
        double denom = r0 - r0 * 0.0;
        double sumF = 0.0;
        for (int j = 0; j < k; ++j) {
            sumF += firstColumn[j + 1] * bPrev[j];
        }
        denom = 1.0 - sumF * firstColumn[1] / firstColumn[0];

        /* 使用标准的Levinson递归公式 */
        /* 先求后向解 */
        double lambda = 0.0;
        for (int j = 0; j < k; ++j) {
            lambda += firstColumn[k - j] * fPrev[j];
        }
        lambda = (firstColumn[k] - lambda);

        /* 计算新分母 */
        double prevDenom = firstColumn[0];
        for (int j = 0; j < k; ++j) {
            prevDenom -= firstColumn[j + 1] * bPrev[k - 1 - j];
        }
        /* 修正: prevDenom = 1 - r_{1:k}^T * b_{k-1} */
        prevDenom = firstColumn[0];
        for (int j = 0; j < k; ++j) {
            prevDenom -= firstColumn[j + 1] * fPrev[k - 1 - j];
        }

        double newDenom = firstColumn[0] - lambda * lambda / prevDenom;
        if (qAbs(newDenom) < 1e-30) break;

        /* 更新前向和后向解 */
        QVector<double> fNew(k + 1);
        QVector<double> bNew(k + 1);

        double ratio = lambda / prevDenom;

        /* f_k = [f_{k-1}; 0] - ratio * [0; b_{k-1}] */
        for (int j = 0; j < k; ++j) {
            fNew[j] = fPrev[j] - ratio * bPrev[k - 1 - j];
        }
        fNew[k] = -ratio;

        /* b_k = [0; b_{k-1}] - ratio * [f_{k-1}; 0] (反向) */
        bNew[0] = -ratio;
        for (int j = 1; j <= k; ++j) {
            bNew[j] = bPrev[j - 1] - ratio * fPrev[k - j];
        }

        fPrev = fNew;
        bPrev = bNew;

        /* 更新解 x_k */
        double gamma = 0.0;
        for (int j = 0; j < k; ++j) {
            gamma += firstColumn[k - j] * x[j];
        }
        gamma = rhs[k] - gamma;

        QVector<double> xNew(k + 1);
        for (int j = 0; j < k; ++j) {
            xNew[j] = x[j] + (gamma / newDenom) * bNew[j];
        }
        xNew[k] = gamma / newDenom;
        x = xNew;
    }

    /* 确保结果维度正确 */
    if (x.size() != n) {
        x.resize(n, 0.0);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(n);
    return x;
}

/** @brief Levinson-Durbin求解Yule-Walker方程
 *  @param autocorr 自相关序列 r[0..n-1]
 *  @return {反射系数数组, 预测误差功率} */
QPair<QVector<double>, double> ToeplitzSolver::levinsonDurbin(
    const QVector<double>& autocorr)
{
    int n = autocorr.size();
    if (n == 0 || qAbs(autocorr[0]) < 1e-30) {
        return {{}, 0.0};
    }

    QVector<double> a(n, 0.0);     ///< AR系数(a[0]=1)
    a[0] = 1.0;

    double error = autocorr[0];     ///< 当前预测误差
    QVector<double> reflections;    ///< 反射系数

    for (int k = 1; k < n; ++k) {
        /* 计算反射系数 */
        double lambda = 0.0;
        for (int j = 0; j < k; ++j) {
            lambda += a[j] * autocorr[k - j];
        }
        lambda = -lambda / error;

        reflections.append(-lambda);

        /* 更新AR系数 */
        for (int j = 1; j <= k / 2; ++j) {
            double aj = a[j];
            double akj = a[k - j];
            a[j] = aj + lambda * akj;
            a[k - j] = akj + lambda * aj;
        }
        if (k % 2 == 0) {
            /* 中间元素特殊处理 */
        }
        a[k] = lambda;

        /* 更新预测误差 */
        error = error * (1.0 - lambda * lambda);
        if (error < 1e-30) break;
    }

    return {reflections, error};
}

/** @brief 重置统计 */
void ToeplitzSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

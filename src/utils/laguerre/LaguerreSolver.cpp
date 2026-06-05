/**
 * @file LaguerreSolver.cpp
 * @brief Laguerre方法求多项式根实现 — 三次收敛单根+deflation全根
 */

#include "utils/laguerre/LaguerreSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LaguerreSolver::LaguerreSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 求值多项式及其一阶、二阶导数 */
void LaguerreSolver::evalPoly(const QVector<double>& coeffs, double x,
                               double& p, double& dp, double& ddp) const
{
    int n = coeffs.size() - 1;
    p = coeffs[0];
    dp = 0.0;
    ddp = 0.0;

    for (int i = 1; i <= n; ++i) {
        ddp = 2.0 * dp + x * ddp;
        dp = p + x * dp;
        p = coeffs[i] + x * p;
    }
}

/** @brief 多项式缩减(去除已知根) */
QVector<double> LaguerreSolver::deflate(const QVector<double>& coeffs,
                                         double root) const
{
    int n = coeffs.size() - 1;
    if (n < 1) return coeffs;

    QVector<double> reduced(n);
    reduced[0] = coeffs[0];

    for (int i = 1; i < n; ++i)
        reduced[i] = coeffs[i] + root * reduced[i - 1];

    return reduced;
}

/** @brief Laguerre迭代求单个根 */
double LaguerreSolver::solve(const QVector<double>& coeffs,
                              double x0, int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    int n = coeffs.size() - 1;
    if (n < 1) {
        emit rootFound(0.0, 0);
        return 0.0;
    }

    /* 常数多项式直接返回0 */
    if (n == 0) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            static_cast<double>(m_stats.totalSolves);
        emit rootFound(0.0, 0);
        return 0.0;
    }

    double x = x0;
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double p, dp, ddp;
        evalPoly(coeffs, x, p, dp, ddp);

        /* 已足够接近根 */
        if (std::abs(p) < tol * (1.0 + std::abs(x)))
            break;

        /* 计算 G = (p'/p)^2 - p''/p */
        double denom = p;
        if (std::abs(denom) < 1e-300) break;

        double denomP = dp / denom;
        double g = denomP * denomP - ddp / denom;
        double sqArg = static_cast<double>(n) * (
            static_cast<double>(n) * g - denomP * denomP);

        double sq = (sqArg >= 0.0) ? std::sqrt(sqArg)
                                   : std::sqrt(-sqArg);

        double d1 = denomP + sq;
        double d2 = denomP - sq;

        /* 选择较大的分母以获得较小的步长 */
        double a = static_cast<double>(n) /
            ((std::abs(d1) > std::abs(d2)) ? d1 : d2);

        if (std::abs(a) < tol * (1.0 + std::abs(x))) break;

        x -= a;
    }

    /* 更新统计 */
    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        static_cast<double>(m_stats.totalSolves);

    emit rootFound(x, iter + 1);
    return x;
}

/** @brief 通过deflation求全部根 */
QVector<double> LaguerreSolver::solveAll(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    int n = coeffs.size() - 1;
    QVector<double> roots;

    if (n < 1) {
        emit rootFound(0.0, 0);
        return roots;
    }

    QVector<double> current = coeffs;

    for (int i = 0; i < n; ++i) {
        int remainingDeg = current.size() - 1;

        if (remainingDeg == 1) {
            /* 一次多项式: a*x + b = 0 => x = -b/a */
            if (std::abs(current[0]) > 1e-300)
                roots.append(-current[1] / current[0]);
            break;
        }

        /* 用不同的初始猜测以提高鲁棒性 */
        double x0 = (i == 0) ? 0.0 : roots.last() + 0.1;
        double root = solve(current, x0, 100, 1e-14);

        /* 精炼根(用原始多项式Newton修正) */
        for (int refine = 0; refine < 5; ++refine) {
            double p, dp, ddp;
            evalPoly(coeffs, root, p, dp, ddp);
            if (std::abs(dp) > 1e-300)
                root -= p / dp;
        }

        roots.append(root);
        current = deflate(current, root);
    }

    /* 更新统计( solve() 内部已更新一次，此处只更新总耗时) */
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        static_cast<double>(m_stats.totalSolves);

    return roots;
}

/** @brief 重置统计 */
void LaguerreSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

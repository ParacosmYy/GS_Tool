/**
 * @file BacktrackingLineSearch.cpp
 * @brief 回溯线搜索实现
 */

#include "utils/backtrack/BacktrackingLineSearch.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
BacktrackingLineSearch::BacktrackingLineSearch(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行回溯线搜索
 *
 * 从初始步长 α₀ 开始，反复以因子 ρ 缩减步长，
 * 直至满足 Armijo 条件: f(x+α·d) ≤ f(x) + c·α·∇f(x)ᵀd。
 */
double BacktrackingLineSearch::search(ObjFunc f, GradFunc grad,
                                      QVector<double> x,
                                      QVector<double> direction,
                                      double alpha0, double rho,
                                      double c)
{
    QElapsedTimer timer;
    timer.start();

    int evals = 0;
    double alpha = alpha0;

    /* 计算初始函数值和方向导数 */
    double f0 = f(x);
    evals++;

    const int n = x.size();
    double dirDeriv = 0.0;
    QVector<double> g = grad(x);
    for (int i = 0; i < n; ++i)
        dirDeriv += g[i] * direction[i];

    /* 回溯循环 */
    const int maxSteps = 64;
    for (int step = 0; step < maxSteps; ++step) {
        /* 试探点 x + α·d */
        QVector<double> xTrial(n);
        for (int i = 0; i < n; ++i)
            xTrial[i] = x[i] + alpha * direction[i];

        double fTrial = f(xTrial);
        evals++;

        /* Armijo 充分下降条件 */
        if (fTrial <= f0 + c * alpha * dirDeriv)
            break;

        alpha *= rho;
    }

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
    emit searchCompleted(alpha, evals);
    return alpha;
}

/** @brief 重置统计 */
void BacktrackingLineSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file NewtonRaphson.cpp
 * @brief Newton-Raphson根求解引擎实现 — 解析/数值微分根查找
 */

#include "utils/newtonroot/NewtonRaphson.h"

#include <QElapsedTimer>
#include <QPair>

/** @brief 构造函数 @param parent 父对象 */
NewtonRaphson::NewtonRaphson(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief 使用解析导数求解f(x)=0
 * @param f 目标函数
 * @param df 解析导数
 * @param x0 初始猜测值
 * @param tol 收敛容差
 * @param maxIter 最大迭代次数
 * @return 求得的根
 */
double NewtonRaphson::solve(std::function<double(double)> f,
                            std::function<double(double)> df,
                            double x0, double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    auto result = iterate(f, df, x0, tol, maxIter);

    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalSolves;

    emit rootFound(result.first, result.second);
    return result.first;
}

/**
 * @brief 使用数值差分近似导数求解f(x)=0
 * @param f 目标函数
 * @param x0 初始猜测值
 * @param tol 收敛容差
 * @param maxIter 最大迭代次数
 * @param h 差分步长
 * @return 求得的根
 */
double NewtonRaphson::solveNumerical(std::function<double(double)> f,
                                     double x0, double tol,
                                     int maxIter, double h)
{
    QElapsedTimer timer;
    timer.start();

    /* 中心差分近似导数 */
    auto df = [f, h](double x) -> double {
        return (f(x + h) - f(x - h)) / (2.0 * h);
    };

    auto result = iterate(f, df, x0, tol, maxIter);

    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalSolves;

    emit rootFound(result.first, result.second);
    return result.first;
}

/**
 * @brief 核心Newton-Raphson迭代
 * @param f 目标函数
 * @param df 导数函数
 * @param x0 初始值
 * @param tol 容差
 * @param maxIter 最大迭代次数
 * @return QPair(根, 迭代次数)
 *
 * 迭代公式: x_{n+1} = x_n - f(x_n) / f'(x_n)
 * 当|f(x_n)| < tol或达到最大迭代次数时停止。
 */
QPair<double, int> NewtonRaphson::iterate(
    std::function<double(double)> f,
    std::function<double(double)> df,
    double x0, double tol, int maxIter)
{
    double x = x0;
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double fx = f(x);
        if (std::abs(fx) < tol) {
            return {x, iter};
        }

        double dfx = df(x);
        if (std::abs(dfx) < 1e-15) {
            /* 导数接近零，避免除零，提前返回 */
            return {x, iter};
        }

        x = x - fx / dfx;
    }

    /* 达到最大迭代次数，返回当前最佳估计 */
    return {x, iter};
}

/** @brief 重置统计信息 */
void NewtonRaphson::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

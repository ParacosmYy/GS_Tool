/**
 * @file Rk2Solver.cpp
 * @brief 二阶Runge-Kutta求解器实现 — Midpoint/Ralston方法
 */

#include "utils/rk2/Rk2Solver.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
Rk2Solver::Rk2Solver(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief Midpoint法求解ODE
 * @param f 右端函数
 * @param y0 初始值
 * @param t0 初始时间
 * @param tf 终止时间
 * @param h 步长
 * @return 解的轨迹
 */
QVector<QPair<double, double>> Rk2Solver::midpoint(
    std::function<double(double, double)> f,
    double y0, double t0, double tf, double h)
{
    QElapsedTimer timer;
    timer.start();

    /* 确保步长为正 */
    double step = std::abs(h);
    if (step < 1e-15) step = 0.01;

    int numSteps = static_cast<int>(std::abs(tf - t0) / step) + 1;
    QVector<QPair<double, double>> solution;
    solution.reserve(numSteps + 1);

    double t = t0;
    double y = y0;
    solution.append({t, y});

    int stepCount = 0;
    while ((t0 < tf && t < tf - step / 2.0) ||
           (t0 > tf && t > tf + step / 2.0))
    {
        /* Midpoint法(RK2中点法) */
        double k1 = f(t, y);
        double k2 = f(t + step / 2.0, y + step / 2.0 * k1);
        y = y + step * k2;
        t = t + step;
        solution.append({t, y});
        ++stepCount;
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalSolves;
    m_stats.totalSteps += static_cast<quint64>(stepCount);
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalSolves;

    emit solveCompleted(stepCount, t);
    return solution;
}

/**
 * @brief Ralston法求解ODE
 * @param f 右端函数
 * @param y0 初始值
 * @param t0 初始时间
 * @param tf 终止时间
 * @param h 步长
 * @return 解的轨迹
 */
QVector<QPair<double, double>> Rk2Solver::ralston(
    std::function<double(double, double)> f,
    double y0, double t0, double tf, double h)
{
    QElapsedTimer timer;
    timer.start();

    /* 确保步长为正 */
    double step = std::abs(h);
    if (step < 1e-15) step = 0.01;

    int numSteps = static_cast<int>(std::abs(tf - t0) / step) + 1;
    QVector<QPair<double, double>> solution;
    solution.reserve(numSteps + 1);

    double t = t0;
    double y = y0;
    solution.append({t, y});

    int stepCount = 0;
    while ((t0 < tf && t < tf - step / 2.0) ||
           (t0 > tf && t > tf + step / 2.0))
    {
        /* Ralston法(最小误差界RK2) */
        double k1 = f(t, y);
        double k2 = f(t + 2.0 * step / 3.0,
                       y + 2.0 * step / 3.0 * k1);
        y = y + step / 4.0 * (k1 + 3.0 * k2);
        t = t + step;
        solution.append({t, y});
        ++stepCount;
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalSolves;
    m_stats.totalSteps += static_cast<quint64>(stepCount);
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalSolves;

    emit solveCompleted(stepCount, t);
    return solution;
}

/** @brief 重置统计信息 */
void Rk2Solver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

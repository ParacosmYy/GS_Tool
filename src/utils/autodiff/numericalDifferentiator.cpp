/**
 * @file numericalDifferentiator.cpp
 * @brief 数值微分引擎实现 — 前向/后向/中心/高阶差分
 */

#include "utils/autodiff/numericalDifferentiator.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
NumericalDifferentiator::NumericalDifferentiator(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief 前向差分求一阶导数
 * @param f 目标函数
 * @param x 求导点
 * @param h 步长
 * @return f'(x)的近似值
 */
double NumericalDifferentiator::forwardDifference(
    std::function<double(double)> f, double x, double h)
{
    QElapsedTimer timer;
    timer.start();

    double result = (f(x + h) - f(x)) / h;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalDifferentiations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalDifferentiations;

    emit differentiationCompleted(result);
    return result;
}

/**
 * @brief 后向差分求一阶导数
 * @param f 目标函数
 * @param x 求导点
 * @param h 步长
 * @return f'(x)的近似值
 */
double NumericalDifferentiator::backwardDifference(
    std::function<double(double)> f, double x, double h)
{
    QElapsedTimer timer;
    timer.start();

    double result = (f(x) - f(x - h)) / h;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalDifferentiations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalDifferentiations;

    emit differentiationCompleted(result);
    return result;
}

/**
 * @brief 中心差分求一阶导数
 * @param f 目标函数
 * @param x 求导点
 * @param h 步长
 * @return f'(x)的近似值
 */
double NumericalDifferentiator::centralDifference(
    std::function<double(double)> f, double x, double h)
{
    QElapsedTimer timer;
    timer.start();

    double result = (f(x + h) - f(x - h)) / (2.0 * h);

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalDifferentiations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalDifferentiations;

    emit differentiationCompleted(result);
    return result;
}

/**
 * @brief 中心差分求二阶导数
 * @param f 目标函数
 * @param x 求导点
 * @param h 步长
 * @return f''(x)的近似值
 */
double NumericalDifferentiator::secondDerivative(
    std::function<double(double)> f, double x, double h)
{
    QElapsedTimer timer;
    timer.start();

    double result = (f(x + h) - 2.0 * f(x) + f(x - h)) / (h * h);

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalDifferentiations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalDifferentiations;

    emit differentiationCompleted(result);
    return result;
}

/** @brief 重置统计信息 */
void NumericalDifferentiator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

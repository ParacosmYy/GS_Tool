/**
 * @file MonteCarlo.cpp
 * @brief 蒙特卡洛积分器实现 — 数值积分与随机采样
 */

#include "utils/prob/MonteCarlo.h"

#include <QtMath>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
MonteCarlo::MonteCarlo(QObject* parent)
    : QObject(parent)
    , m_seed(12345)
    , m_state(12345)
    , m_timeSum(0.0)
{
}

/** @brief 设置随机数种子 @param seed 种子值 */
void MonteCarlo::setSeed(quint64 seed)
{
    m_seed  = seed;
    m_state = seed;
}

/** @brief 一维蒙特卡洛积分 @param fn 被积函数 @param lo 下限 @param hi 上限 @param samples 采样数 @return 估计值 */
double MonteCarlo::integrate(std::function<double(double)> fn,
                             double lo, double hi, int samples)
{
    m_timer.start();

    if (samples <= 0) samples = 1000;
    double range = hi - lo;

    /* Xorshift64 PRNG */
    double sum = 0.0;
    for (int i = 0; i < samples; ++i) {
        double x = lo + uniformRandom() * range;
        sum += fn(x);
    }

    /* 积分 ≈ (b-a) * mean(f(x_i)) */
    double result = range * sum / static_cast<double>(samples);

    ++m_stats.totalIntegrations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIntegrations);

    emit integrationCompleted(result, samples);
    return result;
}

/** @brief 二维蒙特卡洛积分 @param fn 被积函数 @param xLo @param xHi @param yLo @param yHi @param samples @return 估计值 */
double MonteCarlo::integrate2D(std::function<double(double, double)> fn,
                               double xLo, double xHi,
                               double yLo, double yHi,
                               int samples)
{
    m_timer.start();

    if (samples <= 0) samples = 1000;
    double area = (xHi - xLo) * (yHi - yLo);

    double sum = 0.0;
    for (int i = 0; i < samples; ++i) {
        double x = xLo + uniformRandom() * (xHi - xLo);
        double y = yLo + uniformRandom() * (yHi - yLo);
        sum += fn(x, y);
    }

    double result = area * sum / static_cast<double>(samples);

    ++m_stats.totalIntegrations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIntegrations);

    emit integrationCompleted(result, samples);
    return result;
}

/** @brief 圆周率估计 @param samples 采样数 @return pi估计值 */
double MonteCarlo::piEstimate(int samples)
{
    m_timer.start();

    if (samples <= 0) samples = 1000;

    /* 在[-1,1]x[-1,1]正方形内投点，统计落在单位圆内的比例 */
    int insideCount = 0;
    for (int i = 0; i < samples; ++i) {
        double x = 2.0 * uniformRandom() - 1.0; /* [-1, 1] */
        double y = 2.0 * uniformRandom() - 1.0;
        if (x * x + y * y <= 1.0) {
            ++insideCount;
        }
    }

    /* pi = 4 * (圆内点数 / 总点数) */
    double pi = 4.0 * static_cast<double>(insideCount)
              / static_cast<double>(samples);

    ++m_stats.totalIntegrations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIntegrations);

    emit integrationCompleted(pi, samples);
    return pi;
}

/** @brief 期望值计算 E[f(X)] @param fn 被积函数 @param sampler 采样器 @param samples 采样数 @return 期望值 */
double MonteCarlo::expectedValue(std::function<double(double)> fn,
                                 std::function<double()> sampler,
                                 int samples)
{
    m_timer.start();

    if (samples <= 0) samples = 1000;

    double sum = 0.0;
    for (int i = 0; i < samples; ++i) {
        double x = sampler();
        sum += fn(x);
    }

    double result = sum / static_cast<double>(samples);

    ++m_stats.totalIntegrations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIntegrations);

    emit integrationCompleted(result, samples);
    return result;
}

/** @brief 重置统计 */
void MonteCarlo::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
    m_state   = m_seed;
}

/** @brief 生成[0,1)均匀分布随机数(Xorshift64) @return 随机数 */
double MonteCarlo::uniformRandom()
{
    /* Xorshift64 伪随机数生成器 */
    quint64 x = m_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    m_state = x;

    /* 映射到 [0, 1) */
    return static_cast<double>(x >> 11) / 9007199254740992.0;
}

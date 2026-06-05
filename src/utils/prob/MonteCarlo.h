/**
 * @file MonteCarlo.h
 * @brief 蒙特卡洛积分器 — 数值积分与随机采样
 *
 * 功能: 使用蒙特卡洛方法进行数值积分(1D/2D)、圆周率估计、
 *       期望值计算。支持用户自定义被积函数和采样器，
 *       通过伪随机采样逼近积分结果。适用于复杂区域积分、
 *       概率分布期望计算、信号能量估计等场景。
 *
 * 协作: BayesianEstimator(贝叶斯推断) / StatDistribution(分布采样)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <functional>

/**
 * @brief 蒙特卡洛积分器 — 数值积分与随机采样
 *
 * 使用Mersenne Twister伪随机数生成器(PRNG)，
 * 支持可设置种子以保证结果可重现。
 */
class MonteCarlo : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIntegrations = 0;       ///< 累计积分计算次数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MonteCarlo(QObject* parent = nullptr);

    /**
     * @brief 设置随机数种子
     * @param seed 种子值
     */
    void setSeed(quint64 seed);

    /**
     * @brief 一维蒙特卡洛积分
     * @param fn  被积函数 f(x)
     * @param lo  积分下限
     * @param hi  积分上限
     * @param samples 采样点数
     * @return 积分估计值
     */
    double integrate(std::function<double(double)> fn,
                     double lo, double hi, int samples);

    /**
     * @brief 二维蒙特卡洛积分
     * @param fn  被积函数 f(x,y)
     * @param xLo X下限
     * @param xHi X上限
     * @param yLo Y下限
     * @param yHi Y上限
     * @param samples 采样点数
     * @return 积分估计值
     */
    double integrate2D(std::function<double(double, double)> fn,
                       double xLo, double xHi,
                       double yLo, double yHi,
                       int samples);

    /**
     * @brief 圆周率估计(单位圆内投点法)
     * @param samples 采样点数
     * @return pi估计值
     */
    double piEstimate(int samples);

    /**
     * @brief 期望值计算 E[f(X)]
     * @param fn      被积函数 f(x)
     * @param sampler 随机采样器，每次调用返回一个X样本
     * @param samples 采样点数
     * @return 期望值估计
     */
    double expectedValue(std::function<double(double)> fn,
                         std::function<double()> sampler,
                         int samples);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 积分完成信号 @param value 估计值 @param samples 采样数 */
    void integrationCompleted(double value, int samples);

private:
    /**
     * @brief 生成[0,1)均匀分布随机数
     * @return 随机数
     */
    double uniformRandom();

    quint64 m_seed;                  ///< 随机种子
    quint64 m_state;                 ///< PRNG状态

    QElapsedTimer m_timer;           ///< 计时器
    double  m_timeSum;               ///< 累计耗时
    Stats   m_stats;                 ///< 统计信息
};

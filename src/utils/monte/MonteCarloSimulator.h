/**
 * @file MonteCarloSimulator.h
 * @brief 蒙特卡洛模拟器 — 随机采样与统计聚合
 *
 * 功能: 支持均匀/正态/指数分布的随机采样，可配置迭代次数，
 *       对结果进行统计聚合，计算收敛率，用于概率估计和数值积分。
 *
 * 协作: QueueSimulator(排队论) / EntropyCalculator(不确定性分析)
 */
#ifndef MONTECARLOSIMULATOR_H
#define MONTECARLOSIMULATOR_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <functional>
#include <random>

/**
 * @class MonteCarloSimulator
 * @brief 蒙特卡洛随机模拟框架
 */
class MonteCarloSimulator : public QObject {
    Q_OBJECT

public:
    /** 随机分布类型 */
    enum class Distribution {
        Uniform,        ///< 均匀分布 [a, b]
        Normal,         ///< 正态分布 (μ, σ)
        Exponential     ///< 指数分布 (λ)
    };

    /** 模拟统计 */
    struct Stats {
        quint64 totalSimulations = 0;       ///< 总模拟次数
        quint64 totalIterations = 0;        ///< 总迭代次数
        double  avgResult = 0.0;            ///< 平均结果
        double  convergenceRate = 0.0;      ///< 收敛率
    };

    /** 分布参数 */
    struct DistributionParams {
        double paramA = 0.0;                ///< 均匀下界/正态均值/指数λ
        double paramB = 1.0;                ///< 均匀上界/正态标准差(未使用)
    };

    /** 单次模拟结果 */
    struct SimulationResult {
        double mean = 0.0;                  ///< 样本均值
        double stddev = 0.0;                ///< 样本标准差
        double min = 0.0;                   ///< 最小值
        double max = 0.0;                   ///< 最大值
        double confidence95 = 0.0;          ///< 95%置信区间半宽
        quint64 iterations = 0;            ///< 实际迭代次数
    };

    explicit MonteCarloSimulator(QObject* parent = nullptr);

    /** @brief 设置随机分布类型 @param dist 分布类型 */
    void setDistribution(Distribution dist);

    /** @brief 设置分布参数 @param params 分布参数 */
    void setDistributionParams(const DistributionParams& params);

    /** @brief 设置迭代次数 @param count 每次模拟的迭代数 */
    void setIterationCount(quint64 count);

    /** @brief 设置随机种子 @param seed 种子值 */
    void setSeed(quint32 seed);

    /**
     * @brief 执行模拟
     * @param estimator 估计函数，输入随机样本，输出结果
     * @return 模拟结果
     */
    SimulationResult simulate(std::function<double(double)> estimator);

    /**
     * @brief 多维模拟
     * @param estimator 多维估计函数
     * @param dimensions 维度数
     * @return 模拟结果
     */
    SimulationResult simulateMultiDim(
        std::function<double(const QVector<double>&)> estimator,
        int dimensions);

    /** @brief 生成随机样本 @param count 样本数 @return 样本向量 */
    QVector<double> generateSamples(int count);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 模拟完成 @param result 模拟结果 */
    void simulationCompleted(const SimulationResult& result);

    /** @brief 进度通知 @param progress 0.0~1.0 */
    void progressChanged(double progress);

private:
    double sample();    ///< 生成单个随机样本
    void ensureGenerator(); ///< 确保随机引擎已初始化

    Distribution m_distribution;         ///< 分布类型
    DistributionParams m_params;         ///< 分布参数
    quint64 m_iterationCount;            ///< 迭代次数
    quint32 m_seed;                      ///< 随机种子
    bool m_seedSet;                      ///< 是否设置了种子
    bool m_generatorReady;               ///< 随机引擎就绪标志
    std::mt19937 m_generator;            ///< 随机数引擎

    Stats m_stats;
    double m_resultSum;                  ///< 结果累计
    double m_convergenceCount;           ///< 收敛计数
};

#endif // MONTECARLOSIMULATOR_H

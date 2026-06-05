/**
 * @file GeneticOptimizer.h
 * @brief 遗传算法优化器 — 进化计算
 *
 * 功能: 使用遗传算法求解优化问题，支持实数编码、
 *       锦标赛选择、均匀交叉和高斯变异。
 *
 * 协作: SimulatedAnnealing(单点) / PidController(参数整定)
 */
#ifndef GENETICOPTIMIZER_H
#define GENETICOPTIMIZER_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 遗传算法优化器
 */
class GeneticOptimizer : public QObject {
    Q_OBJECT

public:
    /** @brief 适应度函数 */
    using FitnessFunc = std::function<double(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalGenerations = 0;   ///< 累计代数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 优化参数 */
    struct Parameters {
        int populationSize = 50;        ///< 种群大小
        int maxGenerations = 100;       ///< 最大代数
        double crossoverRate = 0.8;     ///< 交叉率
        double mutationRate = 0.1;      ///< 变异率
        double mutationStd = 0.1;       ///< 变异标准差
        int tournamentSize = 3;         ///< 锦标赛大小
        double elitismRatio = 0.1;      ///< 精英保留比例
    };

    /** @brief 优化结果 */
    struct Result {
        QVector<double> bestIndividual; ///< 最优个体
        double bestFitness = 0.0;       ///< 最优适应度
        QVector<double> fitnessHistory; ///< 每代最优适应度
        int generations = 0;            ///< 实际代数
    };

    explicit GeneticOptimizer(QObject* parent = nullptr);

    /** @brief 执行遗传算法优化(最小化)
     *  @param fitness 适应度函数(越小越好)
     *  @param lowerBounds 参数下界
     *  @param upperBounds 参数上界
     *  @param params 优化参数
     *  @return 优化结果 */
    Result optimize(const FitnessFunc& fitness,
                    const QVector<double>& lowerBounds,
                    const QVector<double>& upperBounds,
                    const Parameters& params);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 代完成 @param generation 代号 @param bestFitness 最优适应度 */
    void generationCompleted(int generation, double bestFitness);

    /** @brief 优化完成 @param bestFitness 最优适应度 @param generations 总代数 */
    void optimizationCompleted(double bestFitness, int generations);

private:
    /** @brief 初始化种群 */
    QVector<QVector<double>> initPopulation(
        int popSize, const QVector<double>& lower,
        const QVector<double>& upper);

    /** @brief 锦标赛选择 */
    int tournamentSelect(const QVector<double>& fitnesses,
                         int tournamentSize);

    /** @brief 均匀交叉 */
    QVector<double> crossover(const QVector<double>& p1,
                              const QVector<double>& p2,
                              double rate);

    /** @brief 高斯变异 */
    void mutate(QVector<double>& individual,
                const QVector<double>& lower,
                const QVector<double>& upper,
                double rate, double std);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // GENETICOPTIMIZER_H

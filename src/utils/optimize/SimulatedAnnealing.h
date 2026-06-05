/**
 * @file SimulatedAnnealing.h
 * @brief 模拟退火优化器 — 全局优化启发式
 *
 * 功能: 使用模拟退火算法求解连续/离散优化问题。
 *       支持自定义目标函数、邻域生成和冷却策略。
 *
 * 协作: GeneticOptimizer(进化) / PidController(控制优化)
 */
#ifndef SIMULATEDANNEALING_H
#define SIMULATEDANNEALING_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 模拟退火优化器
 */
class SimulatedAnnealing : public QObject {
    Q_OBJECT

public:
    /** @brief 目标函数: 解 -> 适应度(越小越好) */
    using ObjectiveFunc = std::function<double(const QVector<double>&)>;

    /** @brief 邻域生成: 当前解 -> 新解 */
    using NeighborFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIterations = 0;    ///< 累计迭代次数
        quint64 totalAcceptances = 0;   ///< 累计接受次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 优化结果 */
    struct Result {
        QVector<double> bestSolution;   ///< 最优解
        double bestFitness = 0.0;       ///< 最优适应度
        int iterations = 0;             ///< 总迭代次数
        int acceptances = 0;            ///< 接受次数
    };

    explicit SimulatedAnnealing(QObject* parent = nullptr);

    /** @brief 执行模拟退火
     *  @param initialSolution 初始解
     *  @param objective 目标函数
     *  @param neighbor 邻域生成函数
     *  @param initialTemp 初始温度
     *  @param coolingRate 冷却系数(0.9~0.999)
     *  @param maxIterations 最大迭代次数
     *  @return 优化结果 */
    Result optimize(const QVector<double>& initialSolution,
                    const ObjectiveFunc& objective,
                    const NeighborFunc& neighbor,
                    double initialTemp = 1000.0,
                    double coolingRate = 0.995,
                    int maxIterations = 10000);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 迭代完成 @param iteration 迭代号 @param temp 当前温度 @param fitness 当前适应度 */
    void iterationCompleted(int iteration, double temp, double fitness);

    /** @brief 优化完成 @param bestFitness 最优适应度 */
    void optimizationCompleted(double bestFitness);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // SIMULATEDANNEALING_H

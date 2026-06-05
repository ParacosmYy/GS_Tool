/**
 * @file SimulatedAnnealing2.h
 * @brief 高级模拟退火优化器 — 自适应冷却策略
 *
 * 功能: 实现带自适应冷却、温度重启和自适应邻域的模拟退火算法。
 *       适用于连续空间和组合优化问题, 支持自定义能量函数和
 *       邻域生成函数。
 *
 * 协作: TrendPredictor(趋势预测优化) / DataOptimizer(参数优化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <functional>

/**
 * @brief 高级模拟退火优化器 — 自适应冷却策略
 */
class SimulatedAnnealing2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalOptimizations = 0;   ///< 累计优化次数
        double  bestEnergyFound = 1e18;   ///< 历史最优能量
        double  avgIterations = 0.0;      ///< 平均迭代次数
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    /**
     * @brief 构造模拟退火优化器
     * @param initialTemp 初始温度(默认100.0)
     * @param coolingRate 冷却率(0-1, 默认0.95)
     * @param minTemp 最低温度阈值(默认1e-6)
     * @param maxIterations 每个温度最大迭代次数(默认1000)
     * @param parent 父对象
     */
    explicit SimulatedAnnealing2(double initialTemp = 100.0,
                                  double coolingRate = 0.95,
                                  double minTemp = 1e-6,
                                  int maxIterations = 1000,
                                  QObject* parent = nullptr);

    /**
     * @brief 设置能量(目标)函数
     * @param fn 能量函数, 接受状态向量返回能量值
     *
     * 能量值越小代表解越优。必须在optimize()前设置。
     */
    void setEnergyFunction(std::function<double(const QVector<double>&)> fn);

    /**
     * @brief 设置邻域生成函数
     * @param fn 邻域函数, 接受当前状态返回新候选状态
     *
     * 邻域函数决定搜索空间中的移动策略。
     * 若不设置则使用默认高斯扰动。
     */
    void setNeighborFunction(
        std::function<QVector<double>(const QVector<double>&)> fn);

    /**
     * @brief 执行模拟退火优化
     * @param initialState 初始状态向量
     * @return 最优状态向量
     *
     * 从initialState出发搜索能量最小化状态。
     * 必须先调用setEnergyFunction()设置目标函数。
     */
    QVector<double> optimize(const QVector<double>& initialState);

    /**
     * @brief 启用/禁用自适应冷却
     * @param enabled true启用自适应冷却, false使用固定冷却率
     *
     * 自适应模式根据接受率动态调整冷却速度:
     * 接受率高时降温快, 接受率低时降温慢(近似退火重启)。
     */
    void setAdaptiveCooling(bool enabled);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 优化完成 @param bestEnergy 最优能量 @param iterations 迭代次数 */
    void optimizationFinished(double bestEnergy, int iterations);

    /** @brief 温度更新 @param temperature 当前温度 */
    void temperatureUpdated(double temperature);

    /** @brief 找到更优解 @param energy 新能量 @param state 新状态 */
    void betterSolutionFound(double energy, const QVector<double>& state);

private:
    /**
     * @brief 默认邻域生成(高斯扰动)
     * @param current 当前状态
     * @return 扰动后的新状态
     */
    QVector<double> defaultNeighbor(const QVector<double>& current) const;

    /**
     * @brief 计算自适应冷却因子
     * @param acceptanceRate 当前温度下的接受率
     * @return 新的冷却因子
     */
    double adaptiveCoolFactor(double acceptanceRate) const;

    double m_initialTemp;          ///< 初始温度
    double m_coolingRate;          ///< 冷却率
    double m_minTemp;              ///< 最低温度阈值
    int    m_maxIterations;        ///< 每温度最大迭代次数
    bool   m_adaptiveCooling;      ///< 是否启用自适应冷却

    std::function<double(const QVector<double>&)> m_energyFn;    ///< 能量函数
    std::function<QVector<double>(const QVector<double>&)> m_neighborFn; ///< 邻域函数

    Stats  m_stats;                ///< 统计信息
    double m_timeSum;              ///< 处理时间累加器
    double m_iterationSum;         ///< 迭代次数累加器
};

/**
 * @file SimulatedAnnealing2.cpp
 * @brief 高级模拟退火优化器实现 — 自适应冷却策略
 */

#include "utils/optim2/SimulatedAnnealing2.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>

/** @brief 构造函数
 *  @param initialTemp 初始温度
 *  @param coolingRate 冷却率
 *  @param minTemp 最低温度阈值
 *  @param maxIterations 每温度最大迭代次数
 *  @param parent 父对象
 */
SimulatedAnnealing2::SimulatedAnnealing2(double initialTemp,
                                         double coolingRate,
                                         double minTemp,
                                         int maxIterations,
                                         QObject* parent)
    : QObject(parent)
    , m_initialTemp(qMax(1e-3, initialTemp))
    , m_coolingRate(qBound(0.5, coolingRate, 0.999))
    , m_minTemp(qMax(1e-12, minTemp))
    , m_maxIterations(qMax(10, maxIterations))
    , m_adaptiveCooling(false)
    , m_timeSum(0.0)
    , m_iterationSum(0.0)
{
}

/** @brief 设置能量函数
 *  @param fn 能量函数
 */
void SimulatedAnnealing2::setEnergyFunction(
    std::function<double(const QVector<double>&)> fn)
{
    m_energyFn = std::move(fn);
}

/** @brief 设置邻域生成函数
 *  @param fn 邻域函数
 */
void SimulatedAnnealing2::setNeighborFunction(
    std::function<QVector<double>(const QVector<double>&)> fn)
{
    m_neighborFn = std::move(fn);
}

/** @brief 启用/禁用自适应冷却
 *  @param enabled 是否启用
 */
void SimulatedAnnealing2::setAdaptiveCooling(bool enabled)
{
    m_adaptiveCooling = enabled;
}

/** @brief 默认邻域生成(高斯扰动)
 *  @param current 当前状态
 *  @return 扰动后的新状态
 */
QVector<double> SimulatedAnnealing2::defaultNeighbor(
    const QVector<double>& current) const
{
    QVector<double> neighbor;
    neighbor.reserve(current.size());
    for (double v : current) {
        /* Box-Muller变换生成高斯扰动 */
        double u1 = QRandomGenerator::global()->generateDouble();
        double u2 = QRandomGenerator::global()->generateDouble();
        u1 = qMax(u1, 1e-15); /* 避免log(0) */
        double gaussian = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
        double step = 0.1 * qAbs(v + 1.0); /* 扰动幅度与当前值相关 */
        neighbor.append(v + gaussian * step);
    }
    return neighbor;
}

/** @brief 计算自适应冷却因子
 *  @param acceptanceRate 当前温度下的接受率
 *  @return 新的冷却因子
 */
double SimulatedAnnealing2::adaptiveCoolFactor(double acceptanceRate) const
{
    /* 接受率高(>0.6): 快速降温
     * 接受率中(0.2~0.6): 标准降温
     * 接受率低(<0.2): 减缓降温避免过早冻结 */
    if (acceptanceRate > 0.6) {
        return qPow(m_coolingRate, 0.5); /* 更快降温 */
    } else if (acceptanceRate < 0.2) {
        return qPow(m_coolingRate, 2.0); /* 更慢降温 */
    }
    return m_coolingRate;
}

/** @brief 执行模拟退火优化
 *  @param initialState 初始状态向量
 *  @return 最优状态向量
 */
QVector<double> SimulatedAnnealing2::optimize(
    const QVector<double>& initialState)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_energyFn || initialState.isEmpty()) {
        return initialState;
    }

    QVector<double> current = initialState;
    double currentEnergy = m_energyFn(current);
    QVector<double> best = current;
    double bestEnergy = currentEnergy;

    double temp = m_initialTemp;
    int totalIter = 0;
    int outerIter = 0;
    const int kMaxOuter = 500; /* 外层最大温度迭代次数 */
    double bestEnergyAtTempStart = bestEnergy;

    while (temp > m_minTemp && outerIter < kMaxOuter) {
        int accepted = 0;

        for (int i = 0; i < m_maxIterations; ++i) {
            /* 生成邻域解 */
            QVector<double> neighbor = m_neighborFn
                ? m_neighborFn(current)
                : defaultNeighbor(current);

            double neighborEnergy = m_energyFn(neighbor);
            double delta = neighborEnergy - currentEnergy;

            /* Metropolis准则: 接受更优解或以概率接受较差解 */
            if (delta < 0 || QRandomGenerator::global()->generateDouble()
                < qExp(-delta / qMax(temp, 1e-15))) {
                current = std::move(neighbor);
                currentEnergy = neighborEnergy;
                ++accepted;

                if (currentEnergy < bestEnergy) {
                    best = current;
                    bestEnergy = currentEnergy;
                    emit betterSolutionFound(bestEnergy, best);
                }
            }
            ++totalIter;
        }

        /* 自适应冷却 */
        double acceptRate = static_cast<double>(accepted)
            / static_cast<double>(m_maxIterations);
        if (m_adaptiveCooling) {
            temp *= adaptiveCoolFactor(acceptRate);
        } else {
            temp *= m_coolingRate;
        }

        /* 温度重启: 若此温度段无改进则适当升温 */
        if (m_adaptiveCooling && bestEnergy >= bestEnergyAtTempStart - 1e-12) {
            temp *= 1.5; /* 局部重启 */
        }
        bestEnergyAtTempStart = bestEnergy;

        emit temperatureUpdated(temp);
        ++outerIter;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalOptimizations;
    if (bestEnergy < m_stats.bestEnergyFound) {
        m_stats.bestEnergyFound = bestEnergy;
    }
    m_iterationSum += static_cast<double>(totalIter);
    m_stats.avgIterations = m_iterationSum
        / static_cast<double>(m_stats.totalOptimizations);
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalOptimizations);

    emit optimizationFinished(bestEnergy, totalIter);
    return best;
}

/** @brief 重置统计信息 */
void SimulatedAnnealing2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterationSum = 0.0;
}

/**
 * @file SimulatedAnnealing.cpp
 * @brief 模拟退火优化器实现
 */

#include "utils/optimize/SimulatedAnnealing.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
SimulatedAnnealing::SimulatedAnnealing(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 执行模拟退火 */
SimulatedAnnealing::Result SimulatedAnnealing::optimize(
    const QVector<double>& initialSolution,
    const ObjectiveFunc& objective,
    const NeighborFunc& neighbor,
    double initialTemp, double coolingRate, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    QVector<double> current = initialSolution;
    double currentFit = objective(current);
    QVector<double> best = current;
    double bestFit = currentFit;
    double temp = initialTemp;

    int accepted = 0;
    int iterations = maxIterations;

    for (int iter = 0; iter < maxIterations; ++iter) {
        QVector<double> candidate = neighbor(current);
        double candFit = objective(candidate);
        double delta = candFit - currentFit;

        /* 接受准则: 更优直接接受，更差以exp(-delta/T)概率接受 */
        bool accept = false;
        if (delta <= 0.0) {
            accept = true;
        } else if (temp > 1e-15) {
            double prob = qExp(-delta / temp);
            if (QRandomGenerator::global()->generateDouble() < prob)
                accept = true;
        }

        if (accept) {
            current = candidate;
            currentFit = candFit;
            ++accepted;
        }

        if (currentFit < bestFit) {
            best = current;
            bestFit = currentFit;
        }

        /* 冷却 */
        temp *= coolingRate;

        /* 定期发送进度 */
        if (iter % 100 == 0) {
            emit iterationCompleted(iter, temp, bestFit);
        }

        if (temp < 1e-10) {
            iterations = iter + 1;
            break;
        }
    }

    result.bestSolution = best;
    result.bestFitness = bestFit;
    result.iterations = iterations;
    result.acceptances = accepted;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalIterations += static_cast<quint64>(iterations);
    m_stats.totalAcceptances += static_cast<quint64>(accepted);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIterations);

    emit optimizationCompleted(bestFit);
    return result;
}

/** @brief 重置统计 */
void SimulatedAnnealing::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

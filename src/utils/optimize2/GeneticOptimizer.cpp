/**
 * @file GeneticOptimizer.cpp
 * @brief 遗传算法优化器实现
 */

#include "utils/optimize2/GeneticOptimizer.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
GeneticOptimizer::GeneticOptimizer(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 执行遗传算法 */
GeneticOptimizer::Result GeneticOptimizer::optimize(
    const FitnessFunc& fitness,
    const QVector<double>& lowerBounds,
    const QVector<double>& upperBounds,
    const Parameters& params)
{
    QElapsedTimer timer;
    timer.start();

    Result result;
    int n = lowerBounds.size();
    int popSize = params.populationSize;
    int eliteCount = qMax(1, static_cast<int>(popSize * params.elitismRatio));

    /* 初始化种群 */
    QVector<QVector<double>> population = initPopulation(
        popSize, lowerBounds, upperBounds);

    /* 评估适应度 */
    QVector<double> fitnesses(popSize);
    int bestIdx = 0;
    for (int i = 0; i < popSize; ++i) {
        fitnesses[i] = fitness(population[i]);
        if (fitnesses[i] < fitnesses[bestIdx]) bestIdx = i;
    }

    result.bestIndividual = population[bestIdx];
    result.bestFitness = fitnesses[bestIdx];

    for (int gen = 0; gen < params.maxGenerations; ++gen) {
        /* 按适应度排序 */
        QVector<int> indices(popSize);
        for (int i = 0; i < popSize; ++i) indices[i] = i;
        std::sort(indices.begin(), indices.end(),
                  [&fitnesses](int a, int b) {
                      return fitnesses[a] < fitnesses[b];
                  });

        /* 精英保留 */
        QVector<QVector<double>> newPop;
        newPop.reserve(popSize);
        for (int i = 0; i < eliteCount; ++i)
            newPop.append(population[indices[i]]);

        /* 选择+交叉+变异 */
        while (static_cast<int>(newPop.size()) < popSize) {
            int p1 = tournamentSelect(fitnesses, params.tournamentSize);
            int p2 = tournamentSelect(fitnesses, params.tournamentSize);

            QVector<double> child = crossover(population[p1], population[p2],
                                              params.crossoverRate);
            mutate(child, lowerBounds, upperBounds,
                   params.mutationRate, params.mutationStd);
            newPop.append(child);
        }

        population = newPop;

        /* 重新评估 */
        bestIdx = 0;
        for (int i = 0; i < popSize; ++i) {
            fitnesses[i] = fitness(population[i]);
            if (fitnesses[i] < fitnesses[bestIdx]) bestIdx = i;
        }

        if (fitnesses[bestIdx] < result.bestFitness) {
            result.bestIndividual = population[bestIdx];
            result.bestFitness = fitnesses[bestIdx];
        }

        result.fitnessHistory.append(result.bestFitness);

        if (gen % 10 == 0)
            emit generationCompleted(gen, result.bestFitness);
    }

    result.generations = params.maxGenerations;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalGenerations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalGenerations);

    emit optimizationCompleted(result.bestFitness, result.generations);
    return result;
}

/** @brief 初始化种群 */
QVector<QVector<double>> GeneticOptimizer::initPopulation(
    int popSize, const QVector<double>& lower,
    const QVector<double>& upper)
{
    QVector<QVector<double>> pop;
    pop.reserve(popSize);
    int n = lower.size();

    for (int i = 0; i < popSize; ++i) {
        QVector<double> individual(n);
        for (int j = 0; j < n; ++j) {
            double r = QRandomGenerator::global()->generateDouble();
            individual[j] = lower[j] + r * (upper[j] - lower[j]);
        }
        pop.append(individual);
    }
    return pop;
}

/** @brief 锦标赛选择 */
int GeneticOptimizer::tournamentSelect(const QVector<double>& fitnesses,
                                        int tournamentSize)
{
    int popSize = fitnesses.size();
    int best = QRandomGenerator::global()->bounded(popSize);
    for (int i = 1; i < tournamentSize; ++i) {
        int candidate = QRandomGenerator::global()->bounded(popSize);
        if (fitnesses[candidate] < fitnesses[best])
            best = candidate;
    }
    return best;
}

/** @brief 均匀交叉 */
QVector<double> GeneticOptimizer::crossover(const QVector<double>& p1,
                                             const QVector<double>& p2,
                                             double rate)
{
    int n = p1.size();
    QVector<double> child(n);
    for (int i = 0; i < n; ++i) {
        if (QRandomGenerator::global()->generateDouble() < rate)
            child[i] = p1[i];
        else
            child[i] = p2[i];
    }
    return child;
}

/** @brief 高斯变异 */
void GeneticOptimizer::mutate(QVector<double>& individual,
                               const QVector<double>& lower,
                               const QVector<double>& upper,
                               double rate, double std)
{
    int n = individual.size();
    for (int i = 0; i < n; ++i) {
        if (QRandomGenerator::global()->generateDouble() < rate) {
            /* Box-Muller变换生成高斯随机数 */
            double u1 = QRandomGenerator::global()->generateDouble();
            double u2 = QRandomGenerator::global()->generateDouble();
            u1 = qMax(u1, 1e-15);
            double z = qSqrt(-2.0 * qLn(u1)) * qCos(2.0 * M_PI * u2);
            individual[i] += z * std * (upper[i] - lower[i]);
            individual[i] = qBound(lower[i], individual[i], upper[i]);
        }
    }
}

/** @brief 重置统计 */
void GeneticOptimizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

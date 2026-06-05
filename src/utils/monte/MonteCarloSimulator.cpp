/**
 * @file MonteCarloSimulator.cpp
 * @brief 蒙特卡洛模拟器实现
 */

#include "utils/monte/MonteCarloSimulator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <numeric>
#include <algorithm>

MonteCarloSimulator::MonteCarloSimulator(QObject* parent)
    : QObject(parent)
    , m_distribution(Distribution::Uniform)
    , m_iterationCount(10000)
    , m_seed(0)
    , m_seedSet(false)
    , m_generatorReady(false)
    , m_generator(std::random_device{}())
    , m_resultSum(0.0)
    , m_convergenceCount(0.0)
{
    ensureGenerator();
}

void MonteCarloSimulator::setDistribution(Distribution dist)
{
    m_distribution = dist;
}

void MonteCarloSimulator::setDistributionParams(const DistributionParams& params)
{
    m_params = params;
}

void MonteCarloSimulator::setIterationCount(quint64 count)
{
    m_iterationCount = qMax(1ULL, count);
}

void MonteCarloSimulator::setSeed(quint32 seed)
{
    m_seed = seed;
    m_seedSet = true;
    m_generatorReady = false; // 需要重新初始化
    ensureGenerator();
}

void MonteCarloSimulator::ensureGenerator()
{
    if (!m_generatorReady) {
        if (m_seedSet) {
            m_generator.seed(m_seed);
        } else {
            m_generator.seed(std::random_device{}());
        }
        m_generatorReady = true;
    }
}

double MonteCarloSimulator::sample()
{
    ensureGenerator();

    switch (m_distribution) {
    case Distribution::Uniform: {
        std::uniform_real_distribution<double> dist(m_params.paramA, m_params.paramB);
        return dist(m_generator);
    }
    case Distribution::Normal: {
        std::normal_distribution<double> dist(m_params.paramA, m_params.paramB);
        return dist(m_generator);
    }
    case Distribution::Exponential: {
        double lambda = qMax(1e-10, m_params.paramA);
        std::exponential_distribution<double> dist(lambda);
        return dist(m_generator);
    }
    }
    return 0.0;
}

QVector<double> MonteCarloSimulator::generateSamples(int count)
{
    QVector<double> samples;
    samples.reserve(count);
    for (int i = 0; i < count; ++i) {
        samples.append(sample());
    }
    return samples;
}

MonteCarloSimulator::SimulationResult MonteCarloSimulator::simulate(
    std::function<double(double)> estimator)
{
    QElapsedTimer timer;
    timer.start();

    SimulationResult result;
    result.iterations = m_iterationCount;

    double sum = 0.0;
    double sumSq = 0.0;
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    /* 使用Welford在线算法计算均值和方差 */
    double onlineMean = 0.0;
    double onlineM2 = 0.0;

    quint64 progressInterval = qMax(1ULL, m_iterationCount / 100);

    for (quint64 i = 0; i < m_iterationCount; ++i) {
        double x = sample();
        double y = estimator(x);
        sum += y;

        /* Welford更新 */
        double delta = y - onlineMean;
        onlineMean += delta / static_cast<double>(i + 1);
        double delta2 = y - onlineMean;
        onlineM2 += delta * delta2;

        if (y < minVal) minVal = y;
        if (y > maxVal) maxVal = y;

        /* 进度通知(每1%发一次) */
        if ((i + 1) % progressInterval == 0) {
            emit progressChanged(static_cast<double>(i + 1) / m_iterationCount);
        }
    }

    result.mean = onlineMean;
    double variance = (m_iterationCount > 1)
        ? onlineM2 / static_cast<double>(m_iterationCount - 1) : 0.0;
    result.stddev = qSqrt(variance);
    result.min = minVal;
    result.max = maxVal;
    /* 95%置信区间半宽: z * σ / √n */
    result.confidence95 = 1.96 * result.stddev / qSqrt(static_cast<double>(m_iterationCount));

    /* 更新统计 */
    ++m_stats.totalSimulations;
    m_stats.totalIterations += m_iterationCount;
    m_resultSum += result.mean;
    m_stats.avgResult = m_resultSum / m_stats.totalSimulations;

    /* 收敛判断: 后半段均值与前半段均值差异 < 1% */
    if (result.stddev > 1e-15) {
        double cv = result.stddev / qAbs(result.mean + 1e-15);
        if (cv < 0.01) m_convergenceCount += 1.0;
    }
    m_stats.convergenceRate = (m_stats.totalSimulations > 0)
        ? m_convergenceCount / m_stats.totalSimulations : 0.0;

    Q_UNUSED(timer);
    emit simulationCompleted(result);
    return result;
}

MonteCarloSimulator::SimulationResult MonteCarloSimulator::simulateMultiDim(
    std::function<double(const QVector<double>&)> estimator,
    int dimensions)
{
    QElapsedTimer timer;
    timer.start();

    SimulationResult result;
    result.iterations = m_iterationCount;

    double onlineMean = 0.0;
    double onlineM2 = 0.0;
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    quint64 progressInterval = qMax(1ULL, m_iterationCount / 100);

    for (quint64 i = 0; i < m_iterationCount; ++i) {
        /* 为每个维度生成样本 */
        QVector<double> point;
        point.reserve(dimensions);
        for (int d = 0; d < dimensions; ++d) {
            point.append(sample());
        }

        double y = estimator(point);

        double delta = y - onlineMean;
        onlineMean += delta / static_cast<double>(i + 1);
        double delta2 = y - onlineMean;
        onlineM2 += delta * delta2;

        if (y < minVal) minVal = y;
        if (y > maxVal) maxVal = y;

        if ((i + 1) % progressInterval == 0) {
            emit progressChanged(static_cast<double>(i + 1) / m_iterationCount);
        }
    }

    result.mean = onlineMean;
    double variance = (m_iterationCount > 1)
        ? onlineM2 / static_cast<double>(m_iterationCount - 1) : 0.0;
    result.stddev = qSqrt(variance);
    result.min = minVal;
    result.max = maxVal;
    result.confidence95 = 1.96 * result.stddev / qSqrt(static_cast<double>(m_iterationCount));

    /* 更新统计 */
    ++m_stats.totalSimulations;
    m_stats.totalIterations += m_iterationCount;
    m_resultSum += result.mean;
    m_stats.avgResult = m_resultSum / m_stats.totalSimulations;

    if (result.stddev > 1e-15) {
        double cv = result.stddev / qAbs(result.mean + 1e-15);
        if (cv < 0.01) m_convergenceCount += 1.0;
    }
    m_stats.convergenceRate = (m_stats.totalSimulations > 0)
        ? m_convergenceCount / m_stats.totalSimulations : 0.0;

    Q_UNUSED(timer);
    emit simulationCompleted(result);
    return result;
}

void MonteCarloSimulator::resetStatistics()
{
    m_stats = Stats{};
    m_resultSum = 0.0;
    m_convergenceCount = 0.0;
}

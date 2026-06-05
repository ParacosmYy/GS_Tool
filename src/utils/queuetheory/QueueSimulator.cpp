/**
 * @file QueueSimulator.cpp
 * @brief 排队论模拟器实现
 */

#include "utils/queuetheory/QueueSimulator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <functional>

QueueSimulator::QueueSimulator(QObject* parent)
    : QObject(parent)
    , m_modelType(ModelType::MM1)
    , m_arrivalRate(1.0)
    , m_serviceRate(2.0)
    , m_serverCount(1)
    , m_duration(1000.0)
    , m_waitTimeSum(0.0)
    , m_queueLengthSum(0.0)
    , m_utilizationSum(0.0)
{}

void QueueSimulator::setModelType(ModelType type)
{
    m_modelType = type;
}

void QueueSimulator::setArrivalRate(double rate)
{
    m_arrivalRate = qMax(1e-10, rate);
}

void QueueSimulator::setServiceRate(double rate)
{
    m_serviceRate = qMax(1e-10, rate);
}

void QueueSimulator::setServerCount(int count)
{
    m_serverCount = qMax(1, count);
}

void QueueSimulator::setSimulationDuration(double duration)
{
    m_duration = qMax(1.0, duration);
}

QueueSimulator::SimulationResult QueueSimulator::simulate()
{
    QElapsedTimer timer;
    timer.start();

    SimulationResult result;
    switch (m_modelType) {
    case ModelType::MM1:
        result = simulateMM1();
        break;
    case ModelType::MMc:
        result = simulateMMc();
        break;
    }

    /* 累计统计 */
    ++m_stats.totalSimulations;
    m_waitTimeSum += result.waitTime;
    m_queueLengthSum += result.queueLength;
    m_utilizationSum += result.utilization;
    m_stats.avgWaitTime = m_waitTimeSum / m_stats.totalSimulations;
    m_stats.avgQueueLength = m_queueLengthSum / m_stats.totalSimulations;
    m_stats.utilizationRate = m_utilizationSum / m_stats.totalSimulations;

    Q_UNUSED(timer);
    emit simulationCompleted(result);
    return result;
}

QVector<QueueSimulator::SimulationResult> QueueSimulator::simulateBatch(int count)
{
    QVector<SimulationResult> results;
    results.reserve(count);
    for (int i = 0; i < count; ++i) {
        results.append(simulate());
    }
    emit batchCompleted(results);
    return results;
}

QueueSimulator::SimulationResult QueueSimulator::theoreticalMetrics() const
{
    /* 辅助lambda: 计算阶乘 */
    auto factorial = [](int n) -> double {
        double result = 1.0;
        for (int i = 2; i <= n; ++i) result *= i;
        return result;
    };

    SimulationResult r;
    double lambda = m_arrivalRate;
    double mu = m_serviceRate;

    if (m_modelType == ModelType::MM1) {
        /* M/M/1 理论公式 */
        double rho = lambda / mu;
        if (rho >= 1.0) rho = 0.9999; // 避免发散
        r.utilization = rho;
        r.waitTime = rho / (mu - lambda);
        r.queueLength = (rho * rho) / (1.0 - rho);
        r.systemTime = 1.0 / (mu - lambda);
        r.throughput = lambda;
    } else {
        /* M/M/c 理论公式 */
        int c = m_serverCount;
        double rho = lambda / (c * mu);
        if (rho >= 1.0) rho = 0.9999;
        r.utilization = rho;

        /* Erlang-C 公式计算队列概率 P_Q */
        double sumTerms = 0.0;
        for (int k = 0; k < c; ++k) {
            double term = qPow(lambda / mu, k) / factorial(k);
            sumTerms += term;
        }
        double lastTerm = qPow(lambda / mu, c) / factorial(c);
        double pZeroInv = sumTerms + lastTerm / (1.0 - rho);
        double pQ = (lastTerm / (1.0 - rho)) / pZeroInv;

        r.waitTime = pQ / (c * mu - lambda);
        r.queueLength = pQ * rho / (1.0 - rho);
        r.systemTime = r.waitTime + 1.0 / mu;
        r.throughput = lambda;
    }
    return r;
}

QueueSimulator::SimulationResult QueueSimulator::simulateMM1()
{
    SimulationResult result;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> arrivalDist(m_arrivalRate);
    std::exponential_distribution<double> serviceDist(m_serviceRate);

    double currentTime = 0.0;
    double totalWait = 0.0;
    int completedCustomers = 0;
    double nextFreeTime = 0.0;
    double queueLengthTimeIntegral = 0.0;
    int currentQueue = 0;

    while (currentTime < m_duration) {
        /* 生成下一个到达 */
        double interArrival = arrivalDist(gen);
        currentTime += interArrival;
        if (currentTime >= m_duration) break;

        /* 计算等待时间 */
        double waitTime = qMax(0.0, nextFreeTime - currentTime);
        double serviceTime = serviceDist(gen);

        /* 队列长度时间积分 */
        double prevEventTime = currentTime - interArrival;
        queueLengthTimeIntegral += currentQueue * (currentTime - prevEventTime);
        if (waitTime > 0) {
            ++currentQueue;
        }

        nextFreeTime = currentTime + waitTime + serviceTime;
        totalWait += waitTime;
        ++completedCustomers;

        if (waitTime <= 0) {
            currentQueue = qMax(0, currentQueue - 1);
        }
    }

    result.waitTime = (completedCustomers > 0) ? totalWait / completedCustomers : 0.0;
    result.queueLength = (m_duration > 0) ? queueLengthTimeIntegral / m_duration : 0.0;
    result.utilization = m_arrivalRate / m_serviceRate;
    result.systemTime = result.waitTime + 1.0 / m_serviceRate;
    result.throughput = static_cast<double>(completedCustomers) / m_duration;

    return result;
}

QueueSimulator::SimulationResult QueueSimulator::simulateMMc()
{
    SimulationResult result;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<double> arrivalDist(m_arrivalRate);
    std::exponential_distribution<double> serviceDist(m_serviceRate);

    int c = m_serverCount;
    QVector<double> serverFreeTimes(c, 0.0); // 每个服务台下次空闲时间

    double currentTime = 0.0;
    double totalWait = 0.0;
    int completedCustomers = 0;
    double queueLengthTimeIntegral = 0.0;
    int currentQueue = 0;

    while (currentTime < m_duration) {
        double interArrival = arrivalDist(gen);
        double prevTime = currentTime;
        currentTime += interArrival;
        if (currentTime >= m_duration) break;

        /* 找到最早空闲的服务台 */
        double minFreeTime = serverFreeTimes[0];
        for (int i = 1; i < c; ++i) {
            if (serverFreeTimes[i] < minFreeTime)
                minFreeTime = serverFreeTimes[i];
        }

        double waitTime = qMax(0.0, minFreeTime - currentTime);
        queueLengthTimeIntegral += currentQueue * (currentTime - prevTime);

        double serviceTime = serviceDist(gen);

        /* 分配到服务台 */
        int assignedServer = 0;
        double assignedFreeTime = serverFreeTimes[0];
        for (int i = 1; i < c; ++i) {
            if (serverFreeTimes[i] < assignedFreeTime) {
                assignedFreeTime = serverFreeTimes[i];
                assignedServer = i;
            }
        }

        if (waitTime > 0) {
            ++currentQueue;
        }
        serverFreeTimes[assignedServer] = currentTime + waitTime + serviceTime;
        totalWait += waitTime;
        ++completedCustomers;

        /* 简化队列减少逻辑 */
        currentQueue = qMax(0, currentQueue - 1);
    }

    result.waitTime = (completedCustomers > 0) ? totalWait / completedCustomers : 0.0;
    result.queueLength = (m_duration > 0) ? queueLengthTimeIntegral / m_duration : 0.0;
    result.utilization = m_arrivalRate / (static_cast<double>(c) * m_serviceRate);
    result.systemTime = result.waitTime + 1.0 / m_serviceRate;
    result.throughput = static_cast<double>(completedCustomers) / m_duration;

    return result;
}

void QueueSimulator::resetStatistics()
{
    m_stats = Stats{};
    m_waitTimeSum = 0.0;
    m_queueLengthSum = 0.0;
    m_utilizationSum = 0.0;
}

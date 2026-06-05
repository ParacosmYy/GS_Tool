/**
 * @file QueueSimulator.h
 * @brief 排队论模拟器 — M/M/1 和 M/M/c 队列模型仿真
 *
 * 功能: 支持泊松到达/指数服务的经典排队模型，计算等待时间、
 *       队列长度、利用率等关键指标，用于通信和系统性能分析。
 *
 * 协作: DataPipeline(管道延迟分析) / PerformanceMonitor(性能评估)
 */
#ifndef QUEUESIMULATOR_H
#define QUEUESIMULATOR_H

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @class QueueSimulator
 * @brief 基于排队论的队列系统模拟器
 */
class QueueSimulator : public QObject {
    Q_OBJECT

public:
    /** 队列模型类型 */
    enum class ModelType {
        MM1,    ///< M/M/1 单服务台队列
        MMc     ///< M/M/c 多服务台队列
    };

    /** 仿真统计 */
    struct Stats {
        quint64 totalSimulations = 0;       ///< 总仿真次数
        double  avgWaitTime = 0.0;          ///< 平均等待时间
        double  avgQueueLength = 0.0;       ///< 平均队列长度
        double  utilizationRate = 0.0;      ///< 服务台利用率
    };

    /** 单次仿真结果 */
    struct SimulationResult {
        double waitTime = 0.0;              ///< 等待时间
        double queueLength = 0.0;           ///< 稳态队列长度
        double utilization = 0.0;           ///< 利用率
        double systemTime = 0.0;            ///< 系统逗留时间
        double throughput = 0.0;            ///< 吞吐量
    };

    explicit QueueSimulator(QObject* parent = nullptr);

    /** @brief 设置模型类型 @param type 模型类型 */
    void setModelType(ModelType type);

    /** @brief 设置到达率(lambda) @param rate 到达率(请求/秒) */
    void setArrivalRate(double rate);

    /** @brief 设置服务率(mu) @param rate 服务率(请求/秒) */
    void setServiceRate(double rate);

    /** @brief 设置服务台数量(M/M/c模型) @param count 服务台数量 */
    void setServerCount(int count);

    /** @brief 设置仿真时长 @param duration 仿真时长(秒) */
    void setSimulationDuration(double duration);

    /** @brief 执行一次仿真 @return 仿真结果 */
    SimulationResult simulate();

    /** @brief 批量仿真 @param count 仿真次数 @return 结果列表 */
    QVector<SimulationResult> simulateBatch(int count);

    /** @brief 计算理论稳态指标 @return 理论结果 */
    SimulationResult theoreticalMetrics() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 仿真完成 @param result 仿真结果 */
    void simulationCompleted(const SimulationResult& result);

    /** @brief 批量仿真完成 @param results 结果列表 */
    void batchCompleted(const QVector<SimulationResult>& results);

private:
    SimulationResult simulateMM1();
    SimulationResult simulateMMc();

    ModelType m_modelType;              ///< 模型类型
    double m_arrivalRate;               ///< 到达率 λ
    double m_serviceRate;               ///< 服务率 μ
    int m_serverCount;                  ///< 服务台数量 c
    double m_duration;                  ///< 仿真时长

    Stats m_stats;
    double m_waitTimeSum;               ///< 等待时间累计
    double m_queueLengthSum;            ///< 队列长度累计
    double m_utilizationSum;            ///< 利用率累计
};

#endif // QUEUESIMULATOR_H

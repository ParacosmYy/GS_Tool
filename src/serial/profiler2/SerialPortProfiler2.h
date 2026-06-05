/**
 * @file SerialPortProfiler2.h
 * @brief 增强版串口性能分析器 -- 详细时序统计/数据吞吐量模式/端口健康报告
 *
 * 相比 profiler/ 版本，增强功能包括:
 *   - 发送/接收方向独立的时序统计(最小/最大/平均延迟 + 抖动)
 *   - 周期性吞吐量采样历史(ThroughputSample时间线)
 *   - 发送/接收独立峰值/平均速率追踪
 *   - 帧级别计数(Tx/Rx帧数)
 *   - QVariantMap报告生成(便于序列化或UI展示)
 *
 * 协作关系:
 *   - 数据源(recordTx/recordRx) → SerialPortProfiler2 → 报告/信号
 *   - recordTxTiming/recordRxTiming 由上层协议栈在测量到延迟时调用
 *   - onSampleTimer() 由QTimer周期触发，采样当前吞吐量
 */
#ifndef SERIALPORTPROFILER2_H
#define SERIALPORTPROFILER2_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QList>
#include <QElapsedTimer>
#include <QTimer>

/**
 * @brief 时序统计 -- 单方向延迟的分布摘要
 */
struct TimingStats {
    double minLatencyMs = 0.0;      ///< 最小延迟(ms)
    double maxLatencyMs = 0.0;      ///< 最大延迟(ms)
    double avgLatencyMs = 0.0;      ///< 平均延迟(ms)
    double jitterMs = 0.0;          ///< 抖动/标准差(ms)
    quint64 sampleCount = 0;        ///< 采样次数
};

/**
 * @brief 吞吐量采样点 -- 某一时刻的速率快照
 */
struct ThroughputSample {
    qint64 timestampMs = 0;         ///< 采样时刻(相对于profiling开始的毫秒数)
    double bytesPerSecond = 0.0;    ///< 该采样间隔内的字节速率(B/s)
    int bytesInInterval = 0;        ///< 该采样间隔内的字节数
};

/**
 * @brief 完整画像统计 -- 对一次profiling会话的全部统计摘要
 */
struct ProfileStats {
    quint64 totalTxBytes = 0;       ///< 发送总字节数
    quint64 totalRxBytes = 0;       ///< 接收总字节数
    quint64 totalTxFrames = 0;      ///< 发送总帧数
    quint64 totalRxFrames = 0;      ///< 接收总帧数
    double peakTxRate = 0.0;        ///< 发送峰值速率(B/s)
    double peakRxRate = 0.0;        ///< 接收峰值速率(B/s)
    double avgTxRate = 0.0;         ///< 发送平均速率(B/s)
    double avgRxRate = 0.0;         ///< 接收平均速率(B/s)
    TimingStats txTiming;           ///< 发送方向时序统计
    TimingStats rxTiming;           ///< 接收方向时序统计
    quint64 profilingDurationMs = 0; ///< 分析会话持续时间(ms)
};

/**
 * @brief 增强版串口性能分析器 -- 详细时序/吞吐量/健康报告
 *
 * 使用方式:
 *   1. start() 开始性能采集
 *   2. recordTx()/recordRx() 记录发送/接收数据
 *   3. recordTxTiming()/recordRxTiming() 记录延迟测量值
 *   4. throughputHistory() 获取吞吐量时间线
 *   5. generateReport() 生成完整报告
 *   6. stop() 结束采集，最终统计可用
 */
class SerialPortProfiler2 : public QObject {
    Q_OBJECT

public:
    /** @brief 构造增强版串口性能分析器 @param parent 父对象 */
    explicit SerialPortProfiler2(QObject* parent = nullptr);

    /** @brief 析构，停止采样定时器 */
    ~SerialPortProfiler2() override;

    /** @brief 开始性能采集，启动定时器并重置计数器 */
    void start();

    /** @brief 停止性能采集，停止定时器并冻结统计 */
    void stop();

    /** @brief 查询是否正在采集 @return true 表示正在分析中 */
    bool isRunning() const;

    /**
     * @brief 记录发送数据
     * @param data 发送的字节数组
     */
    void recordTx(const QByteArray& data);

    /**
     * @brief 记录接收数据
     * @param data 接收的字节数组
     */
    void recordRx(const QByteArray& data);

    /**
     * @brief 记录一次发送延迟测量值
     * @param latencyMs 延迟毫秒数
     */
    void recordTxTiming(double latencyMs);

    /**
     * @brief 记录一次接收延迟测量值
     * @param latencyMs 延迟毫秒数
     */
    void recordRxTiming(double latencyMs);

    /**
     * @brief 获取吞吐量采样历史
     * @param maxSamples 最多返回的采样点数(默认100)
     * @return 最近maxSamples个ThroughputSample
     */
    QList<ThroughputSample> throughputHistory(int maxSamples = 100) const;

    /**
     * @brief 生成QVariantMap格式的完整性能报告
     * @return 包含所有profiling指标的映射表
     */
    QVariantMap generateReport() const;

    /** @brief 获取当前统计快照 @return ProfileStats副本 */
    ProfileStats stats() const;

    /** @brief 重置所有统计数据和采样历史 */
    void resetStatistics();

signals:
    /** @brief 性能采集已开始 */
    void profilingStarted();

    /** @brief 性能采集已停止 */
    void profilingStopped();

    /**
     * @brief 吞吐量更新信号(每次采样定时器触发时发射)
     * @param txRate 当前发送速率(B/s)
     * @param rxRate 当前接收速率(B/s)
     */
    void throughputUpdated(double txRate, double rxRate);

private:
    /**
     * @brief 采样定时器回调 -- 快照当前吞吐量并追加到历史
     *
     * 计算自上次采样以来的字节增量，转换为速率，
     * 追加ThroughputSample到m_throughputHistory(限制1000条)。
     */
    void onSampleTimer();

    /**
     * @brief 增量更新时序统计(运行中计算min/max/avg/jitter)
     * @param stats 待更新的时序统计结构
     * @param latency 本次延迟测量值(ms)
     * @param sum 延迟累计和(引用，会被更新)
     * @param sumSq 延迟平方累计和(引用，会被更新)
     */
    void updateTimingStats(TimingStats& stats, double latency,
                           double& sum, double& sumSq);

    bool m_running = false;                     ///< 是否正在采集
    QElapsedTimer m_profilerTimer;              ///< 高精度计时器(测量profiling总时长)
    QTimer m_sampleTimer;                       ///< 周期采样定时器(默认1000ms)

    QList<ThroughputSample> m_throughputHistory; ///< 吞吐量采样历史(上限1000条)
    int m_currentIntervalBytes = 0;             ///< 当前采样间隔内累计字节数(Tx+Rx)
    double m_currentRate = 0.0;                 ///< 当前采样间隔内的瞬时速率(B/s)

    ProfileStats m_stats;                       ///< 完整画像统计

    /* 时序累加器 -- 用于运行中计算avg和jitter(标准差) */
    double m_sumTxLatency = 0.0;                ///< Tx延迟累计和
    double m_sumRxLatency = 0.0;                ///< Rx延迟累计和
    double m_sumTxLatencySq = 0.0;              ///< Tx延迟平方累计和
    double m_sumRxLatencySq = 0.0;              ///< Rx延迟平方累计和
};

#endif // SERIALPORTPROFILER2_H

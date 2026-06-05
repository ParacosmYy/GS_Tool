/**
 * @file DataFlowMeter.h
 * @brief 数据流量计 — 实时数据流测量与流量画像
 *
 * 实时吞吐量(字节/秒、包/秒)、滑动窗口移动平均、
 * 突发检测、空闲检测、流量画像(constant/bursty/periodic/idle)。
 * TX/RX双向独立统计，采样间隔可配置。
 */
#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

/** @brief 数据流量计 — 实时双向数据流测量与画像分类 */
class DataFlowMeter : public QObject {
    Q_OBJECT

public:
    /** @brief 数据流方向 */
    enum class Direction {
        Rx,  ///< 接收方向
        Tx   ///< 发送方向
    };
    Q_ENUM(Direction)

    /** @brief 移动平均窗口大小(秒) */
    enum class AverageWindow {
        Sec1  = 1,   ///< 1 秒窗口
        Sec5  = 5,   ///< 5 秒窗口
        Sec30 = 30,  ///< 30 秒窗口
        Sec60 = 60   ///< 60 秒窗口
    };
    Q_ENUM(AverageWindow)

    /** @brief 流量画像类型 */
    enum class TrafficProfile {
        Constant,  ///< 恒定流量 — 速率波动小
        Bursty,    ///< 突发流量 — 偶尔出现高速尖峰
        Periodic,  ///< 周期流量 — 规律性重复模式
        Idle       ///< 空闲 — 近期几乎无数据
    };
    Q_ENUM(TrafficProfile)

    /** @brief 全局统计摘要 */
    struct Stats {
        quint64 totalBytesRx      = 0;  ///< 累计接收字节总数
        quint64 totalBytesTx      = 0;  ///< 累计发送字节总数
        quint64 totalPacketsRx    = 0;  ///< 累计接收包总数
        quint64 totalPacketsTx    = 0;  ///< 累计发送包总数
        double  peakThroughputBps = 0.0; ///< 峰值吞吐量(Bytes/s)
        double  avgThroughputBps  = 0.0; ///< 平均吞吐量(Bytes/s)
        quint64 totalBursts       = 0;  ///< 检测到的突发次数
        quint64 totalIdlePeriods  = 0;  ///< 检测到的空闲次数
        quint64 samplingCount     = 0;  ///< 总采样次数
    };

    /**
     * @brief 构造数据流量计
     * @param parent 父对象
     */
    explicit DataFlowMeter(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DataFlowMeter() override;

    // ── 数据输入 ──

    /** @brief 喂入字节数据 @param count 字节数 @param dir 方向(Rx/Tx) */
    void feedBytes(int count, Direction dir);

    // ── 吞吐量查询 ──

    /** @brief 当前合并(Rx+Tx)吞吐量 @return 移动平均平滑后的吞吐量(Bytes/s) */
    double getThroughput() const;

    /** @brief 所有指标 QVariantMap 快照(可直接用于UI绑定) */
    QVariantMap getMetrics() const;

    // ── 配置 ──

    /** @brief 设置移动平均窗口 @param window 窗口大小 */
    void setAverageWindow(AverageWindow window);
    AverageWindow averageWindow() const { return m_avgWindow; }
    void setSamplingInterval(int intervalMs);
    int samplingInterval() const { return m_samplingIntervalMs; }

    // ── 统计 ──

    Stats stats() const;
    void resetStatistics();

signals:
    void throughputChanged(double bps);        ///< 吞吐量变化(Bytes/s)
    void burstDetected(double bps);            ///< 突发检测
    void idleDetected();                       ///< 空闲检测
    void trafficProfileChanged(const QString &profile); ///< 画像变更

private slots:
    void onSampleTick(); ///< 定时器回调: 采样/计算/检测

private:
    struct Sample {
        qint64  timestampMs = 0;  ///< 采样时间戳(ms)
        double  bytesPerSec = 0.0; ///< 吞吐量(Bytes/s)
        int     packets     = 0;   ///< 数据包数
    };

    void pushSample(const Sample &sample);
    double calcMovingAverage(qint64 nowMs) const;
    bool detectBurst(double instantBps, double movingAvg) const;
    void updateTrafficProfile(qint64 nowMs);
    static QString profileToString(TrafficProfile profile);

    QTimer        m_sampleTimer;
    QElapsedTimer m_elapsed;
    AverageWindow m_avgWindow        = AverageWindow::Sec5;
    int           m_samplingIntervalMs = 1000;
    TrafficProfile m_currentProfile   = TrafficProfile::Idle;
    int     m_periodBytesRx = 0, m_periodBytesTx = 0;
    int     m_periodPacketsRx = 0, m_periodPacketsTx = 0;
    double  m_throughputRx = 0.0, m_throughputTx = 0.0;
    double  m_movingAvg = 0.0, m_peakBps = 0.0;
    qint64  m_lastDataTimeMs = 0;
    bool    m_isIdle = true;
    QList<Sample> m_samples;
    quint64 m_totalBytesRx = 0, m_totalBytesTx = 0;
    quint64 m_totalPacketsRx = 0, m_totalPacketsTx = 0;
    quint64 m_totalBursts = 0, m_totalIdlePeriods = 0, m_samplingCount = 0;

    static constexpr int    kMaxSamples       = 3600;
    static constexpr double kBurstRatio       = 3.0;
    static constexpr int    kIdleThresholdMs  = 5000;
    static constexpr int    kProfileWindowSec = 30;
};

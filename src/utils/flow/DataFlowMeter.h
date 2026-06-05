/**
 * @file DataFlowMeter.h
 * @brief 数据流量计 -- 实时数据流测量与流量画像
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供实时吞吐量测量(字节/秒、包/秒)、可配置滑动窗口移动平均、
 * 突发检测、空闲检测和流量画像分类(constant/bursty/periodic/idle)。
 * TX/RX 双向独立统计，支持采样间隔配置和完整指标快照输出。
 */

#ifndef DATAFLOWMETER_H
#define DATAFLOWMETER_H

#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariantMap>

/**
 * @class DataFlowMeter
 * @brief 数据流量计，实时测量双向数据流并提供流量画像
 *
 * 核心设计：
 * - feedBytes() 喂数据，定时器自动采样计算速率
 * - 移动平均窗口可配置(1/5/30/60秒)
 * - 突发检测: 当瞬时吞吐量超过移动平均的 kBurstRatio 倍时触发
 * - 空闲检测: 连续 kIdleThresholdMs 毫秒无数据时触发
 * - 流量画像: 基于 recent 窗口内的方差分类为 constant/bursty/periodic/idle
 * - getMetrics() 输出 QVariantMap 便于 UI 绑定和序列化
 */
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

    // ── 数据输入接口 ──

    /**
     * @brief 喂入字节数据
     *
     * 每次调用计为一次数据包到达。字节数累计到当前采样周期，
     * 在下次定时器采样时计算吞吐量。
     *
     * @param count 本次数据字节数
     * @param dir 数据流方向(Rx/Tx)
     */
    void feedBytes(int count, Direction dir);

    // ── 吞吐量查询接口 ──

    /**
     * @brief 获取当前合并(Rx+Tx)吞吐量
     * @return 移动平均平滑后的吞吐量(Bytes/s)
     */
    double getThroughput() const;

    /**
     * @brief 获取所有当前指标的 QVariantMap 快照
     *
     * 包含: throughputBps, throughputRx, throughputTx, packetsPerSec,
     * peakBps, avgBps, totalBytesRx, totalBytesTx, totalPacketsRx,
     * totalPacketsTx, burstCount, idleCount, trafficProfile, samplingCount。
     *
     * @return 指标映射表，可直接用于 UI 绑定或 JSON 序列化
     */
    QVariantMap getMetrics() const;

    // ── 配置接口 ──

    /** @brief 设置移动平均窗口大小 @param window 窗口大小枚举 */
    void setAverageWindow(AverageWindow window);

    /** @brief 获取当前移动平均窗口大小 @return 窗口大小枚举 */
    AverageWindow averageWindow() const { return m_avgWindow; }

    /** @brief 设置采样间隔(毫秒) @param intervalMs 采样间隔，默认 1000ms */
    void setSamplingInterval(int intervalMs);

    /** @brief 获取当前采样间隔 @return 采样间隔(ms) */
    int samplingInterval() const { return m_samplingIntervalMs; }

    // ── 统计接口 ──

    /** @brief 获取全局统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有计数器、历史和统计信息(不影响配置) */
    void resetStatistics();

signals:
    /**
     * @brief 吞吐量变化信号，每次采样后发射
     * @param bps 当前合并吞吐量(Bytes/s)
     */
    void throughputChanged(double bps);

    /**
     * @brief 突发检测信号 — 瞬时速率超过移动平均的阈值倍数
     * @param bps 触发突发时的瞬时吞吐量(Bytes/s)
     */
    void burstDetected(double bps);

    /**
     * @brief 空闲检测信号 — 连续无数据超过空闲阈值
     */
    void idleDetected();

    /**
     * @brief 流量画像变更信号
     * @param profile 新的画像名称("constant"/"bursty"/"periodic"/"idle")
     */
    void trafficProfileChanged(const QString &profile);

private slots:
    /** @brief 定时器回调：采样、计算速率、检测突发/空闲、更新画像 */
    void onSampleTick();

private:
    /**
     * @brief 采样点结构
     */
    struct Sample {
        qint64  timestampMs = 0;  ///< 采样时间戳(ms since epoch)
        double  bytesPerSec = 0.0; ///< 此周期吞吐量(Bytes/s)
        int     packets     = 0;   ///< 此周期数据包数
    };

    /** @brief 向采样缓冲区推入一个采样点 @param sample 采样数据 */
    void pushSample(const Sample &sample);

    /**
     * @brief 计算移动平均吞吐量
     * @param nowMs 当前时间戳(ms)
     * @return 窗口内平均吞吐量(Bytes/s)
     */
    double calcMovingAverage(qint64 nowMs) const;

    /**
     * @brief 检测突发：瞬时速率超过移动平均的 kBurstRatio 倍
     * @param instantBps 瞬时吞吐量
     * @param movingAvg 移动平均吞吐量
     * @return true 检测到突发
     */
    bool detectBurst(double instantBps, double movingAvg) const;

    /**
     * @brief 更新流量画像分类
     * @param nowMs 当前时间戳(ms)
     */
    void updateTrafficProfile(qint64 nowMs);

    /**
     * @brief 将 TrafficProfile 枚举转为字符串
     * @param profile 画像枚举值
     * @return 画像名称
     */
    static QString profileToString(TrafficProfile profile);

    // ── 定时器 ──
    QTimer        m_sampleTimer;             ///< 采样定时器
    QElapsedTimer m_elapsed;                 ///< 运行计时器

    // ── 配置 ──
    AverageWindow m_avgWindow        = AverageWindow::Sec5; ///< 移动平均窗口
    int           m_samplingIntervalMs = 1000;              ///< 采样间隔(ms)
    TrafficProfile m_currentProfile   = TrafficProfile::Idle; ///< 当前流量画像

    // ── 当前采样周期累计 ──
    int     m_periodBytesRx = 0;   ///< 当前周期 RX 累计字节
    int     m_periodBytesTx = 0;   ///< 当前周期 TX 累计字节
    int     m_periodPacketsRx = 0; ///< 当前周期 RX 累计包数
    int     m_periodPacketsTx = 0; ///< 当前周期 TX 累计包数

    // ── 速率 ──
    double  m_throughputRx   = 0.0; ///< 最新 RX 吞吐量(Bytes/s)
    double  m_throughputTx   = 0.0; ///< 最新 TX 吞吐量(Bytes/s)
    double  m_movingAvg      = 0.0; ///< 移动平均吞吐量(Rx+Tx)
    double  m_peakBps        = 0.0; ///< 峰值吞吐量(Bytes/s)

    // ── 突发检测 ──
    qint64  m_lastDataTimeMs = 0;   ///< 上次收到数据的时间戳(ms)
    bool    m_isIdle         = true; ///< 当前是否处于空闲状态

    // ── 采样环形缓冲区 ──
    QList<Sample> m_samples;        ///< 采样历史(时间升序)

    // ── 全局统计计数器 ──
    quint64 m_totalBytesRx      = 0; ///< 累计接收字节总数
    quint64 m_totalBytesTx      = 0; ///< 累计发送字节总数
    quint64 m_totalPacketsRx    = 0; ///< 累计接收包总数
    quint64 m_totalPacketsTx    = 0; ///< 累计发送包总数
    quint64 m_totalBursts       = 0; ///< 累计突发次数
    quint64 m_totalIdlePeriods  = 0; ///< 累计空闲次数
    quint64 m_samplingCount     = 0; ///< 累计采样次数

    // ── 常量 ──
    static constexpr int    kMaxSamples       = 3600; ///< 最大采样数(支持60s*60min)
    static constexpr double kBurstRatio       = 3.0;  ///< 突发倍数阈值
    static constexpr int    kIdleThresholdMs  = 5000; ///< 空闲判定阈值(ms)
    static constexpr int    kProfileWindowSec = 30;   ///< 画像分析窗口(秒)
};

#endif // DATAFLOWMETER_H

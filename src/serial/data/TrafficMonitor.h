/**
 * @file TrafficMonitor.h
 * @brief 流量监控器 — 统计和计算串口收发数据速率
 *
 * 记录 TX/RX 字节数，计算实时速率（字节/秒），
 * 维护速率历史数据用于图表显示。
 *
 * 协作关系:
 *   - TrafficMonitorWidget: 显示速率数据和历史曲线
 *   - IConnection: 通过 bytesWritten/dataReceived 信号输入字节数
 */
#ifndef TRAFFICMONITOR_H
#define TRAFFICMONITOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPointF>

/**
 * @brief 流量监控器
 *
 * 累计 TX/RX 字节数，使用定时器周期性计算瞬时速率。
 * 维护速率历史队列供图表绘制。
 */
class TrafficMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TrafficMonitor(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TrafficMonitor() override;

    /** @brief 记录已发送字节数 */
    void recordTxBytes(qint64 bytes);

    /** @brief 记录已接收字节数 */
    void recordRxBytes(qint64 bytes);

    /** @brief 获取当前 RX 速率（字节/秒） */
    double rxRate() const;

    /** @brief 获取当前 TX 速率（字节/秒） */
    double txRate() const;

    /** @brief 获取 RX 速率历史数据（用于绘图） */
    QVector<QPointF> rxRateHistory() const;

    /** @brief 获取 TX 速率历史数据（用于绘图） */
    QVector<QPointF> txRateHistory() const;

    /** @brief 重置所有统计数据 */
    void reset();

    /** @brief 获取累计RX总字节数（重置前） */
    qint64 totalRxBytes() const;

    /** @brief 获取累计TX总字节数（重置前） */
    qint64 totalTxBytes() const;

    /** @brief 获取历史最高RX速率（字节/秒） */
    double peakRxRate() const;

    /** @brief 获取历史最高TX速率（字节/秒） */
    double peakTxRate() const;

    /** @brief 获取速率历史点数 */
    int historySize() const;

    // ── 统计计数器 Getter ──

    /** @brief 获取采样总次数 @return 累计采样点数 */
    quint64 totalSamples() const;

    /** @brief 获取历史最高带宽(RX+TX之和) @return 峰值带宽(bytes/s) */
    double peakBandwidth() const;

    /** @brief 获取监控的字节总数(RX+TX) @return 累计字节数 */
    quint64 totalBytesMonitored() const;

    /** @brief 获取累计接收字节记录次数 @return recordRxBytes调用次数 */
    quint64 totalBytesIn() const;

    /** @brief 获取累计发送字节记录次数 @return recordTxBytes调用次数 */
    quint64 totalBytesOut() const;

    /** @brief 获取累计速率采样次数 @return 历史点追加次数 */
    quint64 totalRateSamples() const;

    /** @brief 获取累计峰值速率刷新事件次数 @return 峰值被刷新的总次数 */
    quint64 totalPeakRateExceededEvents() const;

    /** @brief 重置流量监控统计计数器(不影响速率计算) */
    void resetTrafficStatistics();

signals:
    /**
     * @brief 速率更新信号（周期性发出）
     * @param rxRate 当前 RX 速率（字节/秒）
     * @param txRate 当前 TX 速率（字节/秒）
     */
    void rateUpdated(double rxRate, double txRate);

private slots:
    /** @brief 定时器超时处理，计算瞬时速率 */
    void calculateRates();

private:
    qint64 m_rxBytes = 0;              ///< 累计接收字节数
    qint64 m_txBytes = 0;              ///< 累计发送字节数
    qint64 m_totalRxBytes = 0;         ///< 会话累计RX总字节
    qint64 m_totalTxBytes = 0;         ///< 会话累计TX总字节
    double m_peakRxRate = 0.0;         ///< 历史最高RX速率
    double m_peakTxRate = 0.0;         ///< 历史最高TX速率
    QVector<QPointF> m_rxHistory;      ///< RX 速率历史（x=时间戳, y=速率）
    QVector<QPointF> m_txHistory;      ///< TX 速率历史（x=时间戳, y=速率）
    QTimer* m_calcTimer = nullptr;     ///< 速率计算定时器
    QElapsedTimer m_elapsed;           ///< 经过时间计时器

    // ── 统计计数器 ──
    quint64 m_totalSamples = 0;                ///< 采样总次数(calculateRates调用次数)
    mutable quint64 m_totalBytesIn = 0;        ///< 累计接收字节记录次数(recordRxBytes调用次数)
    mutable quint64 m_totalBytesOut = 0;       ///< 累计发送字节记录次数(recordTxBytes调用次数)
    quint64 m_totalRateSamples = 0;            ///< 累计速率采样次数(历史点追加次数)
    quint64 m_totalPeakRateExceededEvents = 0; ///< 累计峰值速率被刷新的事件次数
};

#endif // TRAFFICMONITOR_H

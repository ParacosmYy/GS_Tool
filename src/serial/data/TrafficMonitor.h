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
    QVector<QPointF> m_rxHistory;      ///< RX 速率历史（x=时间戳, y=速率）
    QVector<QPointF> m_txHistory;      ///< TX 速率历史（x=时间戳, y=速率）
    QTimer* m_calcTimer = nullptr;     ///< 速率计算定时器
    QElapsedTimer m_elapsed;           ///< 经过时间计时器
};

#endif // TRAFFICMONITOR_H

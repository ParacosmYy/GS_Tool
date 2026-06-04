/**
 * @file ConnectionHealthMonitor.h
 * @brief 连接健康监控器 — 实时诊断连接质量
 *
 * 功能: 跟踪连接运行时间/断线次数/数据吞吐量/错误率/延迟，计算综合质量评分(0-100)。
 * 协作: IConnection(接收数据/错误事件) → ConnectionHealthMonitor(分析) → UI/通知
 */
#ifndef CONNECTIONHEALTHMONITOR_H
#define CONNECTIONHEALTHMONITOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QQueue>

/**
 * @brief 连接健康监控器 — 实时评估连接质量并发出通知
 */
class ConnectionHealthMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 连接质量等级 */
    enum class QualityLevel {
        Excellent,  ///< 优秀 (80-100)
        Good,       ///< 良好 (60-79)
        Fair,       ///< 一般 (40-59)
        Poor,       ///< 较差 (20-39)
        Critical    ///< 危险 (0-19)
    };
    Q_ENUM(QualityLevel)

    /** @brief 健康快照 — 某一时刻的连接状态摘要 */
    struct HealthSnapshot {
        double  qualityScore = 0.0;      ///< 质量评分(0-100)
        double  throughputBps = 0.0;     ///< 当前吞吐量(字节/秒)
        double  errorRate = 0.0;         ///< 错误率(错误/分钟)
        qint64  uptimeMs = 0;            ///< 连接运行时间(ms)
        qint64  downtimeMs = 0;          ///< 累计断线时间(ms)
        int     reconnectCount = 0;      ///< 重连次数
        int     consecutiveErrors = 0;   ///< 连续错误次数
    };

    /** @brief 运行统计数据 */
    struct Stats {
        quint64 totalBytesReceived = 0;  ///< 累计接收字节
        quint64 totalBytesSent = 0;      ///< 累计发送字节
        quint64 totalErrors = 0;         ///< 累计错误次数
        quint64 totalReconnects = 0;     ///< 累计重连次数
        quint64 totalSnapshots = 0;      ///< 累计快照次数
        double  peakThroughput = 0.0;    ///< 峰值吞吐量
        double  lowestQuality = 100.0;   ///< 最低质量评分
        double  highestErrorRate = 0.0;  ///< 最高错误率
        qint64  longestUptime = 0;       ///< 最长连接时间(ms)
    };

    /** @brief 构造连接健康监控器 @param parent 父对象 */
    explicit ConnectionHealthMonitor(QObject* parent = nullptr);

    /** @brief 通知接收到数据 @param bytes 字节数 */
    void onDataReceived(qint64 bytes);
    /** @brief 通知发送了数据 @param bytes 字节数 */
    void onDataSent(qint64 bytes);
    /** @brief 通知发生错误 @param description 错误描述 */
    void onError(const QString& description);
    /** @brief 通知连接已建立 */
    void onConnected();
    /** @brief 通知连接已断开 */
    void onDisconnected();

    /** @brief 获取当前健康快照 @return 健康快照 */
    HealthSnapshot currentSnapshot() const;
    /** @brief 获取质量等级 @return 质量等级枚举 */
    QualityLevel qualityLevel() const;
    /** @brief 获取质量评分 @return 0-100分数 */
    double qualityScore() const;
    /** @brief 获取运行统计 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 质量评分变化 @param score 新评分 @param level 新等级 */
    void qualityChanged(double score, QualityLevel level);
    /** @brief 吞吐量更新 @param bytesPerSec 每秒字节数 */
    void throughputUpdated(double bytesPerSec);
    /** @brief 连接状态变化 @param connected 是否已连接 */
    void connectionStateChanged(bool connected);
    /** @brief 健康告警 @param level 等级 @param message 告警消息 */
    void healthAlert(QualityLevel level, const QString& message);

private slots:
    void onSnapshotTimer();

private:
    void calculateQuality();
    double calcThroughputScore() const;
    double calcErrorScore() const;
    double calcStabilityScore() const;
    void recordThroughputSample();

    QTimer m_snapshotTimer;         ///< 快照定时器(1秒)
    QElapsedTimer m_connectionTimer;///< 连接计时器
    QElapsedTimer m_disconnectionTimer;///< 断线计时器
    QQueue<double> m_throughputHistory;///< 吞吐量历史(滑动窗口)

    bool m_connected;               ///< 当前连接状态
    qint64 m_lastBytesReceived;     ///< 上次采样接收字节数
    qint64 m_lastBytesSent;         ///< 上次采样发送字节数
    int m_consecutiveErrors;        ///< 连续错误计数
    double m_currentQuality;        ///< 当前质量评分
    double m_currentThroughput;     ///< 当前吞吐量

    qint64 m_totalDowntimeMs;       ///< 累计断线时间

    Stats m_stats;                  ///< 运行统计
};

#endif // CONNECTIONHEALTHMONITOR_H

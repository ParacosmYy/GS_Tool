/**
 * @file ConnectionMonitor.h
 * @brief 连接状态监控 — 跟踪连接生命周期、延迟、字节统计和重连事件
 *
 * 职责:
 *   1. 管理连接状态机(Disconnected/Connecting/Connected/Reconnecting/Error)
 *   2. 统计连接时长、重连次数、收发字节、延迟
 *   3. 心跳定时器定期更新统计
 *   4. 延迟超过阈值时发出警告信号
 */
#ifndef CONNECTIONMONITOR_H
#define CONNECTIONMONITOR_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QDateTime>

/**
 * @brief 连接状态监控器
 *
 * 跟踪连接生命周期状态转换，累计字节/延迟/重连等统计，
 * 通过信号通知上层状态变化和延迟警告。
 */
class ConnectionMonitor : public QObject {
    Q_OBJECT
public:
    /** @brief 连接状态枚举 */
    enum State { Disconnected, Connecting, Connected, Reconnecting, Error };
    Q_ENUM(State)

    /** @brief 连接统计数据结构 */
    struct Stats {
        State currentState = Disconnected;   ///< 当前连接状态
        qint64 connectedSince = 0;           ///< 本次连接建立时间戳(ms)
        qint64 totalConnectedTime = 0;       ///< 累计连接时长(ms)
        int reconnectCount = 0;              ///< 重连次数
        int errorCount = 0;                  ///< 错误次数
        qint64 bytesSent = 0;                ///< 累计发送字节数
        qint64 bytesReceived = 0;            ///< 累计接收字节数
        double latencyMs = 0.0;              ///< 最新延迟测量值(ms)
        QString lastError;                   ///< 最近一次错误信息
    };

    /** @brief 构造连接监控器 @param parent 父QObject指针 */
    explicit ConnectionMonitor(QObject *parent = nullptr);

    /** @brief 析构连接监控器，停止心跳 */
    ~ConnectionMonitor() override;

    /** @brief 设置连接状态，处理状态转换逻辑 @param state 新的连接状态 */
    void setState(State state);

    /** @brief 获取当前连接状态 @return 当前状态枚举值 */
    State state() const;

    /** @brief 获取连接统计快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 记录发送字节数 @param bytes 发送的字节数 */
    void recordBytesSent(qint64 bytes);

    /** @brief 记录接收字节数 @param bytes 接收的字节数 */
    void recordBytesReceived(qint64 bytes);

    /** @brief 记录延迟测量值 @param ms 延迟毫秒数 */
    void recordLatency(double ms);

    /** @brief 记录一次错误 @param error 错误描述 */
    void recordError(const QString &error);

    /** @brief 开始心跳监控 @param pingIntervalMs 心跳间隔(毫秒)，默认5000ms */
    void startMonitoring(int pingIntervalMs = 5000);

    /** @brief 停止心跳监控 */
    void stopMonitoring();

    /** @brief 重置所有统计数据为默认值 */
    void resetStats();

    /** @brief 获取当前状态的本地化显示文本 @return 状态翻译字符串 */
    QString stateString() const;

signals:
    /** @brief 连接状态变化信号 @param newState 新状态 @param oldState 旧状态 */
    void stateChanged(State newState, State oldState);

    /** @brief 统计信息更新信号 @param stats 最新统计快照 */
    void statsUpdated(const Stats &stats);

    /** @brief 连接丢失信号(Connected→Disconnected) */
    void connectionLost();

    /** @brief 连接恢复信号(Reconnecting→Connected) */
    void connectionRestored();

    /** @brief 延迟超过阈值警告信号 @param ms 当前延迟值(ms) */
    void latencyWarning(double ms);

private:
    /** @brief 心跳定时器超时回调 */
    void onPingTimer();

    Stats m_stats;                          ///< 统计数据
    QTimer *m_pingTimer = nullptr;          ///< 心跳定时器
    int m_pingInterval = 5000;              ///< 心跳间隔(ms)
    double m_latencyThreshold = 500.0;      ///< 延迟警告阈值(ms)
};

#endif // CONNECTIONMONITOR_H

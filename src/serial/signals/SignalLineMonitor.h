/**
 * @file SignalLineMonitor.h
 * @brief 信号线监控器 — 轮询串口信号线状态并通知变化
 *
 * 定期读取 IConnection 的信号线状态（CTS/DSR/DCD/RI/DTR/RTS），
 * 检测变化时发出 signalsChanged 信号。
 *
 * 协作关系:
 *   - IConnection: 提供信号线状态查询接口
 *   - SignalLineWidget: 接收状态更新并刷新 UI 显示
 */
#ifndef SIGNALLINEMONITOR_H
#define SIGNALLINEMONITOR_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "connection/interface/IConnection.h"

/**
 * @brief 信号线监控器
 *
 * 使用定时器周期性轮询 IConnection 的 pinoutSignals() 接口，
 * 当信号线状态发生变化时发出通知。
 */
class SignalLineMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit SignalLineMonitor(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SignalLineMonitor() override;

    /**
     * @brief 开始轮询指定连接的信号线状态
     * @param connection 要监控的连接对象（非空）
     */
    void startPolling(IConnection* connection);

    /** @brief 停止轮询 */
    void stopPolling();

    /** @brief 获取当前信号线状态快照 */
    PinoutSignals currentSignals() const;

    /** @brief 查询是否正在轮询 */
    bool isPolling() const;

    /** @brief 获取信号线变化次数 */
    quint64 changeCount() const;

    /** @brief 获取轮询已运行时长（秒） */
    qint64 pollingDuration() const;

    /** @brief 获取累计轮询次数 */
    quint64 totalPolls() const;

    /** @brief 获取累计无变化轮询次数 */
    quint64 totalIdlePolls() const;

    /** @brief 重置统计计数 */
    void resetStatistics();

signals:
    /**
     * @brief 信号线状态变化通知
     * @param newSignals 最新信号线状态
     */
    void signalsChanged(const PinoutSignals& newSignals);

private slots:
    /** @brief 定时器超时处理，轮询信号线状态 */
    void onTick();

private:
    QTimer* m_pollTimer = nullptr;      ///< 轮询定时器
    PinoutSignals m_current;            ///< 当前缓存的信号线状态
    IConnection* m_connection = nullptr; ///< 被监控的连接对象（不拥有）
    QElapsedTimer m_durationTimer;      ///< 轮询持续时间计时器

    // ---- 统计计数器 ----
    quint64 m_changeCount = 0;          ///< 信号线变化次数
    quint64 m_totalPolls = 0;           ///< 累计轮询次数
    quint64 m_totalIdlePolls = 0;       ///< 累计无变化轮询次数
};

#endif // SIGNALLINEMONITOR_H

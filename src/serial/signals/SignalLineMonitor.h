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

    /** @brief 获取累计信号线变化事件次数 @return 每条线变化+1的累计次数 */
    quint64 totalSignalChanges() const;

    /** @brief 获取累计被监控的信号线总条数 @return 轮询次数×线数的累计 */
    quint64 totalLineMonitored() const;

    /** @brief 获取累计错误事件次数 @return 轮询失败/连接异常等错误累计 */
    quint64 totalErrorEvents() const;

    /** @brief 获取累计DTR信号线变化次数 */
    quint64 totalDtrChanges() const { return m_totalDtrChanges; }
    /** @brief 获取累计RTS信号线变化次数 */
    quint64 totalRtsChanges() const { return m_totalRtsChanges; }
    /** @brief 获取累计CTS信号线变化次数 */
    quint64 totalCtsChanges() const { return m_totalCtsChanges; }
    /** @brief 获取累计DSR信号线变化次数 */
    quint64 totalDsrChanges() const { return m_totalDsrChanges; }
    /** @brief 获取累计DCD信号线变化次数 */
    quint64 totalDcdChanges() const { return m_totalDcdChanges; }
    /** @brief 获取累计RI信号线变化次数 */
    quint64 totalRiChanges() const { return m_totalRiChanges; }
    /** @brief 获取峰值信号变化率(次/秒) */
    quint64 peakChangeRate() const { return m_peakChangeRate; }

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
    quint64 m_totalSignalChanges = 0;   ///< 累计信号线变化事件次数(每条线变化+1)
    quint64 m_totalLineMonitored = 0;   ///< 累计被监控的信号线总条数(每次轮询×线数)
    quint64 m_totalErrorEvents = 0;     ///< 累计错误事件(轮询失败/连接断开等)
    quint64 m_totalDtrChanges = 0;      ///< 累计DTR信号线变化次数
    quint64 m_totalRtsChanges = 0;      ///< 累计RTS信号线变化次数
    quint64 m_totalCtsChanges = 0;      ///< 累计CTS信号线变化次数
    quint64 m_totalDsrChanges = 0;      ///< 累计DSR信号线变化次数
    quint64 m_totalDcdChanges = 0;      ///< 累计DCD信号线变化次数
    quint64 m_totalRiChanges = 0;       ///< 累计RI信号线变化次数
    quint64 m_peakChangeRate = 0;       ///< 峰值信号变化率(次/秒)
    quint64 m_lastSecChanges = 0;       ///< 当前秒内变化次数(用于计算peakChangeRate)
    qint64 m_lastPeakRateSec = 0;       ///< 上一次采样秒数(用于判断秒边界)
};

#endif // SIGNALLINEMONITOR_H

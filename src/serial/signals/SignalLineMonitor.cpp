/**
 * @file SignalLineMonitor.cpp
 * @brief 信号线监控器实现 — 轮询串口信号线状态并通知变化
 */

#include "serial/signals/SignalLineMonitor.h"

/** @brief 构造函数，创建200ms间隔轮询定时器(不自动启动) @param parent 父对象 */
SignalLineMonitor::SignalLineMonitor(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_connection(nullptr)
{
    m_pollTimer->setInterval(200);
    m_pollTimer->setSingleShot(false);
}

/** @brief 析构函数，自动停止轮询 */
SignalLineMonitor::~SignalLineMonitor()
{
    stopPolling();
}

/** @brief 开始轮询指定连接的信号线状态，已在轮询则先停止再重启 @param connection 要监控的连接对象（非空） */
void SignalLineMonitor::startPolling(IConnection* connection)
{
    if (!connection) {
        return;
    }

    // 如果已经在轮询，先停止
    if (isPolling()) {
        stopPolling();
    }

    m_connection = connection;

    // 获取初始状态，避免首次轮询就触发信号
    m_current = m_connection->pinoutSignals();

    connect(m_pollTimer, &QTimer::timeout,
            this, &SignalLineMonitor::onTick);

    m_durationTimer.start();
    m_pollTimer->start();
}

/** @brief 停止轮询，断开信号连接，清除连接引用 */
void SignalLineMonitor::stopPolling()
{
    if (m_pollTimer) {
        m_pollTimer->stop();
        disconnect(m_pollTimer, &QTimer::timeout,
                   this, &SignalLineMonitor::onTick);
    }
    m_connection = nullptr;
}

/** @brief 获取当前信号线状态快照 @return 信号线状态结构体 */
PinoutSignals SignalLineMonitor::currentSignals() const
{
    return m_current;
}

/** @brief 查询是否正在轮询 @return true 正在轮询，false 未轮询 */
bool SignalLineMonitor::isPolling() const
{
    return m_pollTimer != nullptr && m_pollTimer->isActive();
}

/** @brief 定时器超时处理，读取最新信号线状态并与缓存比较，变化时发射signalsChanged信号 */
void SignalLineMonitor::onTick()
{
    if (!m_connection) {
        ++m_totalErrorEvents;
        return;
    }

    ++m_totalPolls;
    // 每次轮询监控6条信号线(CTS/DSR/DCD/RI/DTR/RTS)
    m_totalLineMonitored += 6;

    PinoutSignals latest = m_connection->pinoutSignals();

    // 比较新旧状态 — 逐字段比较避免结构体填充字节干扰
    quint64 changesThisTick = 0;
    if (latest.cts != m_current.cts) { ++m_totalSignalChanges; ++m_totalCtsChanges; ++changesThisTick; }
    if (latest.dsr != m_current.dsr) { ++m_totalSignalChanges; ++m_totalDsrChanges; ++changesThisTick; }
    if (latest.dcd != m_current.dcd) { ++m_totalSignalChanges; ++m_totalDcdChanges; ++changesThisTick; }
    if (latest.ri  != m_current.ri)  { ++m_totalSignalChanges; ++m_totalRiChanges;  ++changesThisTick; }
    if (latest.dtr != m_current.dtr) { ++m_totalSignalChanges; ++m_totalDtrChanges; ++changesThisTick; }
    if (latest.rts != m_current.rts) { ++m_totalSignalChanges; ++m_totalRtsChanges; ++changesThisTick; }

    // 计算peakChangeRate: 累计当前秒内的变化次数，秒边界时更新峰值
    m_lastSecChanges += changesThisTick;
    qint64 currentSec = m_durationTimer.elapsed() / 1000;
    if (currentSec > m_lastPeakRateSec) {
        if (m_lastSecChanges > m_peakChangeRate) {
            m_peakChangeRate = m_lastSecChanges;
        }
        m_lastSecChanges = 0;
        m_lastPeakRateSec = currentSec;
    }

    if (changesThisTick > 0)
    {
        m_current = latest;
        ++m_changeCount;
        emit signalsChanged(m_current);
    } else {
        ++m_totalIdlePolls;
    }
}

// 统计getter/resetStatistics已移至 SignalLineMonitorStats.cpp

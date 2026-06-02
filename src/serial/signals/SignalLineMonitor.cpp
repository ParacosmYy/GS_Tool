/**
 * @file SignalLineMonitor.cpp
 * @brief 信号线监控器实现 — 轮询串口信号线状态并通知变化
 */

#include "serial/signals/SignalLineMonitor.h"

/**
 * @brief 构造函数
 *
 * 创建轮询定时器（200ms 间隔），但不自动启动。
 *
 * @param parent 父对象
 */
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

/**
 * @brief 开始轮询指定连接的信号线状态
 *
 * 保存连接指针，连接定时器超时信号到 onTick 槽，启动定时器。
 * 如果已经在轮询另一个连接，先停止之前的轮询。
 *
 * @param connection 要监控的连接对象（非空）
 */
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

    m_pollTimer->start();
}

/**
 * @brief 停止轮询
 *
 * 停止定时器，断开信号连接，清除连接引用。
 */
void SignalLineMonitor::stopPolling()
{
    if (m_pollTimer) {
        m_pollTimer->stop();
        disconnect(m_pollTimer, &QTimer::timeout,
                   this, &SignalLineMonitor::onTick);
    }
    m_connection = nullptr;
}

/**
 * @brief 获取当前信号线状态快照
 * @return 信号线状态结构体
 */
PinoutSignals SignalLineMonitor::currentSignals() const
{
    return m_current;
}

/**
 * @brief 查询是否正在轮询
 * @return true 正在轮询，false 未轮询
 */
bool SignalLineMonitor::isPolling() const
{
    return m_pollTimer != nullptr && m_pollTimer->isActive();
}

/**
 * @brief 定时器超时处理
 *
 * 从连接读取最新信号线状态，与缓存状态比较。
 * 如果发生变化，更新缓存并发出 signalsChanged 信号。
 */
void SignalLineMonitor::onTick()
{
    if (!m_connection) {
        return;
    }

    PinoutSignals latest = m_connection->pinoutSignals();

    // 比较新旧状态 — 逐字段比较避免结构体填充字节干扰
    if (latest.cts != m_current.cts ||
        latest.dsr != m_current.dsr ||
        latest.dcd != m_current.dcd ||
        latest.ri  != m_current.ri  ||
        latest.dtr != m_current.dtr ||
        latest.rts != m_current.rts)
    {
        m_current = latest;
        emit signalsChanged(m_current);
    }
}

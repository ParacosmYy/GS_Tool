/**
 * @file SignalLineMonitor.cpp
 * @brief 信号线监控器实现 — 骨架文件
 */

#include "serial/signals/SignalLineMonitor.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SignalLineMonitor::SignalLineMonitor(QObject* parent)
    : QObject(parent)
    , m_pollTimer(nullptr)
    , m_connection(nullptr)
{
}

/** @brief 析构函数，自动停止轮询 */
SignalLineMonitor::~SignalLineMonitor()
{
    stopPolling();
}

/**
 * @brief 开始轮询指定连接的信号线状态
 *
 * 创建定时器并启动周期性轮询（默认 200ms 间隔）。
 *
 * @param connection 要监控的连接对象
 */
void SignalLineMonitor::startPolling(IConnection* connection)
{
    Q_UNUSED(connection)
    // TODO: 保存 connection 指针，创建并启动定时器
}

/**
 * @brief 停止轮询
 *
 * 停止并销毁定时器，清除连接引用。
 */
void SignalLineMonitor::stopPolling()
{
    // TODO: 停止定时器，清理资源
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

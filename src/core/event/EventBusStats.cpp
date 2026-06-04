/**
 * @file EventBusStats.cpp
 * @brief 全局事件总线 - 统计与调试接口实现
 *
 * 从 EventBus.cpp 拆分而来，包含订阅者统计、事件名称查询、
 * 计数器访问和统计重置方法。
 */

#include "core/event/EventBus.h"

/** @brief 获取指定事件的订阅者数量 @param eventName 事件名称 @return 订阅者数量 */
int EventBus::subscriberCount(const QString& eventName) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_eventSubscriptions.count(eventName);
}

/** @brief 获取所有已注册的事件名称列表(去重) @return 事件名称字符串列表 */
QStringList EventBus::eventNames() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    QStringList names;
    for (auto it = m_eventSubscriptions.constBegin(); it != m_eventSubscriptions.constEnd(); ++it) {
        if (!names.contains(it.key())) {
            names.append(it.key());
        }
    }
    return names;
}

/** @brief 获取各事件的订阅者数量统计映射 @return 事件名称到订阅者数量的映射 */
QMap<QString, int> EventBus::statistics() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    QMap<QString, int> stats;
    for (auto it = m_eventSubscriptions.constBegin(); it != m_eventSubscriptions.constEnd(); ++it) {
        stats[it.key()]++;
    }
    return stats;
}

/** @brief 获取累计发布的事件总数(同步+异步) @return 事件数 */
quint64 EventBus::totalPublished() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalPublished;
}

/** @brief 获取累计订阅操作总数 @return 订阅数 */
quint64 EventBus::totalSubscriptions() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalSubscriptions;
}

/** @brief 获取累计调用的回调处理器总数 @return 调用次数 */
quint64 EventBus::totalHandlersCalled() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_totalHandlersCalled;
}

/** @brief 重置所有统计计数器(发布/订阅/回调次数) */
void EventBus::resetEventStatistics()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_totalPublished = 0;
    m_totalSubscriptions = 0;
    m_totalHandlersCalled = 0;
}

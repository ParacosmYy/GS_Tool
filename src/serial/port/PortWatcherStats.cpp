/**
 * @file PortWatcherStats.cpp
 * @brief PortWatcher 统计/日志/查询方法 — 从 PortWatcher.cpp 拆分而来
 *
 * 本文件包含:
 *   - 当前状态查询 (currentPorts / currentDevices / deviceInfo)
 *   - 端口事件统计 (totalArrivals / totalRemovals / totalPolls / ...)
 *   - 事件日志管理 (recentEvents / eventLogCapacity / setEventLogCapacity / ...)
 *   - 统计重置 (resetWatcherStatistics)
 *
 * 核心轮询逻辑和构造/析构仍留在 PortWatcher.cpp 中。
 */

#include "serial/port/PortWatcher.h"
#include <algorithm>

// ---- 当前状态查询 ----

/** @brief 获取当前已知的端口名称列表 */
QStringList PortWatcher::currentPorts() const
{
    return m_currentDevices.keys();
}

/** @brief 获取当前所有已知设备的详细信息 @return 设备信息列表 */
QVector<PortDeviceInfo> PortWatcher::currentDevices() const
{
    return m_currentDevices.values().toVector();
}

/** @brief 按端口名查询当前设备详情 @param portName 端口名 @return 设备信息 */
PortDeviceInfo PortWatcher::deviceInfo(const QString& portName) const
{
    return m_currentDevices.value(portName);
}

// ---- 端口事件统计 ----

/** @brief 获取累计检测到的端口新增次数 */
quint64 PortWatcher::totalArrivals() const { return m_totalArrivals; }
/** @brief 获取累计检测到的端口移除次数 */
quint64 PortWatcher::totalRemovals() const { return m_totalRemovals; }
/** @brief 获取累计轮询次数 */
quint64 PortWatcher::totalPolls() const { return m_totalPolls; }
/** @brief 获取累计检测到变化的次数(新增+移除事件合计) */
quint64 PortWatcher::totalChanges() const { return m_totalChanges; }
/** @brief 获取累计端口扫描次数 */
quint64 PortWatcher::totalPortScans() const { return m_totalPortScans; }
/** @brief 获取累计热插拔事件次数 */
quint64 PortWatcher::totalHotplugEvents() const { return m_totalHotplugEvents; }

/** @brief 获取累计端口扫描错误次数 @return 扫描错误总次数 */
quint64 PortWatcher::totalScanErrors() const { return m_totalScanErrors; }

/** @brief 获取最近一次端口变更距现在的毫秒数 @return 毫秒数，无事件返回-1 */
qint64 PortWatcher::msSinceLastChange() const
{
    if (!m_hasLastChange) return -1;
    return m_lastChangeTimer.elapsed();
}

/** @brief 获取当前在线设备数 */
int PortWatcher::onlineDeviceCount() const
{
    return m_currentDevices.size();
}

// ---- 事件日志 ----

/** @brief 获取最近的事件记录列表 @param maxCount 最大返回条数 @return 事件记录列表(按时间倒序) */
QVector<PortEventRecord> PortWatcher::recentEvents(int maxCount) const
{
    if (maxCount <= 0 || maxCount >= m_eventLog.size()) {
        // 返回倒序列表
        QVector<PortEventRecord> reversed = m_eventLog;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
    int start = qMax(0, m_eventLog.size() - maxCount);
    QVector<PortEventRecord> result;
    for (int i = m_eventLog.size() - 1; i >= start; --i) {
        result.append(m_eventLog[i]);
    }
    return result;
}

/** @brief 获取事件日志最大保留条数 */
int PortWatcher::eventLogCapacity() const
{
    return m_eventLogCapacity;
}

/** @brief 设置事件日志最大保留条数 @param capacity 容量，最小10 */
void PortWatcher::setEventLogCapacity(int capacity)
{
    m_eventLogCapacity = qMax(10, capacity);
    // 裁剪超出的旧记录
    while (m_eventLog.size() > m_eventLogCapacity) {
        m_eventLog.removeFirst();
    }
}

/** @brief 重置所有统计计数器和事件日志 */
void PortWatcher::resetWatcherStatistics()
{
    m_totalArrivals = 0;
    m_totalRemovals = 0;
    m_totalPolls = 0;
    m_totalChanges = 0;
    m_totalPortScans = 0;
    m_totalHotplugEvents = 0;
    m_totalScanErrors = 0;
    m_eventLog.clear();
    m_hasLastChange = false;
}

/**
 * @brief 将事件添加到日志(超出容量时移除最旧记录) @param event 事件记录
 */
void PortWatcher::appendEventLog(const PortEventRecord& event)
{
    m_eventLog.append(event);
    while (m_eventLog.size() > m_eventLogCapacity) {
        m_eventLog.removeFirst();
    }
}

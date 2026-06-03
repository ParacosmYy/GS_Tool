/**
 * @file PortWatcher.cpp
 * @brief 串口热插拔检测器实现 - 基于定时轮询检测串口设备的插拔事件
 *
 * 实现原理:
 *   1. start() 时立即拍一次快照作为基准（m_currentPorts）
 *   2. 定时器每 2 秒触发 onTimeout()
 *   3. onTimeout() 查询当前系统端口列表，与快照做差集运算:
 *      - 新列表中有而快照中没有 → portAdded 信号
 *      - 快照中有而新列表中没有 → portRemoved 信号
 *   4. 有变化时更新快照并发射 portsChanged 信号
 */

#include "serial/port/PortWatcher.h"
#include "shared/TimerConstants.h"
#include <QSerialPortInfo>
#include <algorithm>

/**
 * @brief 构造函数
 *
 * 创建定时器并设置默认间隔 2000ms。
 * 定时器的 parent 设为 this，确保随 PortWatcher 一起销毁。
 */
PortWatcher::PortWatcher(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(Timers::kPortPollMs);
    connect(m_timer, &QTimer::timeout, this, &PortWatcher::onTimeout);
}

/** @brief 析构函数 — 停止定时器，m_timer 作为子对象自动销毁 */
PortWatcher::~PortWatcher()
{
    m_timer->stop();
}

/** @brief 启动热插拔检测 — 建立基准快照并启动定时轮询 */
void PortWatcher::start()
{
    if (m_timer->isActive()) {
        return;
    }
    m_currentPorts = queryAvailablePorts();
    m_timer->start();
}

/** @brief 停止热插拔检测 — 停止定时器并清空端口快照 */
void PortWatcher::stop()
{
    m_timer->stop();
    m_currentPorts.clear();
}

/** @brief 检测是否正在运行 */
bool PortWatcher::isRunning() const
{
    return m_timer->isActive();
}

/** @brief 获取轮询间隔（毫秒） */
int PortWatcher::interval() const
{
    return m_timer->interval();
}

/** @brief 设置轮询间隔（毫秒），必须 > 0 */
void PortWatcher::setInterval(int msec)
{
    if (msec > 0) {
        m_timer->setInterval(msec);
    }
}

/** @brief 获取当前已知的端口名称列表 */
QStringList PortWatcher::currentPorts() const
{
    return m_currentPorts;
}

/** @brief 获取累计检测到的端口新增次数 @return 新增总次数 */
quint64 PortWatcher::totalArrivals() const { return m_totalArrivals; }
/** @brief 获取累计检测到的端口移除次数 @return 移除总次数 */
quint64 PortWatcher::totalRemovals() const { return m_totalRemovals; }
/** @brief 获取累计轮询次数 @return 轮询总次数 */
quint64 PortWatcher::totalPolls() const { return m_totalPolls; }
/** @brief 获取累计检测到变化的次数(新增+移除事件合计) @return 变化总次数 */
quint64 PortWatcher::totalChanges() const { return m_totalChanges; }
/** @brief 获取累计端口扫描次数 @return 扫描总次数 */
quint64 PortWatcher::totalPortScans() const { return m_totalPortScans; }
/** @brief 获取累计热插拔事件次数 @return 热插拔事件总次数 */
quint64 PortWatcher::totalHotplugEvents() const { return m_totalHotplugEvents; }

/** @brief 重置所有统计计数器 */
void PortWatcher::resetWatcherStatistics()
{
    m_totalArrivals = 0;
    m_totalRemovals = 0;
    m_totalPolls = 0;
    m_totalChanges = 0;
    m_totalPortScans = 0;
    m_totalHotplugEvents = 0;
}

/**
 * @brief 定时轮询回调（带防抖）
 *
 * 查询当前系统可用端口，与上次快照比较:
 *   - 新增候选: 累计计数达到 kDebounceThreshold 后发射 portAdded
 *   - 移除候选: 累计计数达到 kDebounceThreshold 后发射 portRemoved
 *   - 候选端口恢复原状态 → 清零对应计数器（防抖消除闪烁）
 *   - 确认变化后发射 portsChanged 总信号
 */
void PortWatcher::onTimeout()
{
    const QStringList newPorts = queryAvailablePorts();
    ++m_totalPolls;
    ++m_totalPortScans;
    bool changed = false;

    // ---- 处理新增候选: 在 newPorts 中但不在 m_currentPorts 中 ----
    QStringList confirmedAdds;
    for (const QString& port : newPorts) {
        if (!m_currentPorts.contains(port)) {
            ++m_addedCandidateCount[port];
            if (m_addedCandidateCount[port] >= kDebounceThreshold) {
                emit portAdded(port);
                confirmedAdds.append(port);
                m_addedCandidateCount.remove(port);
                ++m_totalArrivals;
                ++m_totalHotplugEvents;
                changed = true;
            }
        } else {
            m_addedCandidateCount.remove(port);
        }
    }

    // ---- 处理移除候选: 在 m_currentPorts 中但不在 newPorts 中 ----
    QStringList confirmedRemoves;
    for (const QString& port : m_currentPorts) {
        if (!newPorts.contains(port)) {
            ++m_removedCandidateCount[port];
            if (m_removedCandidateCount[port] >= kDebounceThreshold) {
                emit portRemoved(port);
                confirmedRemoves.append(port);
                m_removedCandidateCount.remove(port);
                ++m_totalRemovals;
                ++m_totalHotplugEvents;
                changed = true;
            }
        } else {
            m_removedCandidateCount.remove(port);
        }
    }

    // ---- 清理过期候选: 已恢复原状态的候选计数器 ----
    for (const QString& port : m_addedCandidateCount.keys()) {
        if (m_currentPorts.contains(port)) {
            m_addedCandidateCount.remove(port);
        }
    }

    // ---- 更新快照: 仅追加/移除已确认的端口 ----
    if (changed) {
        ++m_totalChanges;
        for (const QString& port : confirmedAdds) {
            m_currentPorts.append(port);
        }
        for (const QString& port : confirmedRemoves) {
            m_currentPorts.removeAll(port);
        }
        emit portsChanged();
    }
}

/**
 * @brief 查询系统中所有可用串口名称
 * 使用 QSerialPortInfo 枚举所有串口，提取端口名称并排序。
 * @return 排序后的端口名称列表
 */
QStringList PortWatcher::queryAvailablePorts()
{
    QStringList portNames;
    const auto ports = QSerialPortInfo::availablePorts();
    portNames.reserve(ports.size());
    for (const QSerialPortInfo& info : ports) {
        portNames.append(info.portName());
    }
    portNames.sort();
    return portNames;
}


/**
 * @file PortWatcher.cpp
 * @brief 串口热插拔检测器实现 - 增强版，携带设备详情和事件日志
 *
 * 实现原理:
 *   1. start() 时立即拍一次快照作为基准（m_currentDevices）
 *   2. 定时器每 2 秒触发 onTimeout()
 *   3. onTimeout() 查询当前系统端口列表，与快照做差集运算:
 *      - 新列表中有而快照中没有 → deviceAdded 信号(携带设备详情)
 *      - 快照中有而新列表中没有 → deviceRemoved 信号(携带设备详情)
 *   4. 有变化时更新快照、记录事件日志并发射 devicesChanged 信号
 *   5. 防抖: 连续 kDebounceThreshold 次检测到同一变化才确认
 */

#include "serial/port/PortWatcher.h"
#include "shared/TimerConstants.h"
#include <QSerialPortInfo>
#include <algorithm>

// ---- PortDeviceInfo / PortEventRecord 方法 ----

/** @brief 生成单行摘要(端口名+描述+VID:PID) @return 如"COM3 - USB-SERIAL CH340 [1A86:7523]" */
QString PortDeviceInfo::toShortSummary() const
{
    QStringList parts;
    parts << portName;
    if (!description.isEmpty()) parts << description;
    if (!vidHex.isEmpty() && !pidHex.isEmpty())
        parts << QString("[%1:%2]").arg(vidHex, pidHex);
    else if (!vidHex.isEmpty())
        parts << QString("[%1:????]").arg(vidHex);
    return parts.join(" - ");
}

/** @brief 生成事件日志文本 @return 如"[14:32:05] +COM3 CH340 (1A86:7523)" */
QString PortEventRecord::toLogString() const
{
    QString prefix = (type == EventType::Arrival) ? "+" : "-";
    QString time = timestamp.toString("HH:mm:ss");
    QString vidpid;
    if (!device.vidHex.isEmpty() && !device.pidHex.isEmpty())
        vidpid = QString(" (%1:%2)").arg(device.vidHex, device.pidHex);
    return QString("[%1] %2%3%4").arg(time, prefix, device.portName, vidpid);
}

// ---- PortWatcher 构造/析构 ----

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
    m_lastChangeTimer.start();
}

/** @brief 析构函数 — 停止定时器，m_timer 作为子对象自动销毁 */
PortWatcher::~PortWatcher()
{
    m_timer->stop();
}

// ---- 监控控制 ----

/** @brief 启动热插拔检测 — 建立基准快照并启动定时轮询 */
void PortWatcher::start()
{
    if (m_timer->isActive()) {
        return;
    }
    m_currentDevices = queryAvailableDevices();
    m_lastChangeTimer.start();
    m_timer->start();
}

/** @brief 停止热插拔检测 — 停止定时器并清空设备快照 */
void PortWatcher::stop()
{
    m_timer->stop();
    m_currentDevices.clear();
    m_pendingAddDevices.clear();
    m_addedCandidateCount.clear();
    m_removedCandidateCount.clear();
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

// ---- 统计/日志/查询方法已拆分至 PortWatcherStats.cpp ----
// currentPorts / currentDevices / deviceInfo / totalArrivals / totalRemovals /
// totalPolls / totalChanges / totalPortScans / totalHotplugEvents /
// totalScanErrors / msSinceLastChange / onlineDeviceCount / recentEvents /
// eventLogCapacity / setEventLogCapacity / resetWatcherStatistics / appendEventLog

// ---- 定时轮询 ----

// ---- onTimeout/queryAvailableDevices/fromQtInfo见 PortWatcherPoll.cpp ----

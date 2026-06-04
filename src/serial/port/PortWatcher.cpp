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

/**
 * @brief 定时轮询回调（带防抖+设备详情）
 *
 * 查询当前系统可用端口，与上次快照比较:
 *   - 新增候选: 累计计数达到 kDebounceThreshold 后发射 deviceAdded
 *   - 移除候选: 累计计数达到 kDebounceThreshold 后发射 deviceRemoved
 *   - 候选端口恢复原状态 → 清零对应计数器（防抖消除闪烁）
 *   - 确认变化后发射 devicesChanged 总信号并记录事件日志
 */
void PortWatcher::onTimeout()
{
    const QMap<QString, PortDeviceInfo> newDevices = queryAvailableDevices();
    ++m_totalPolls;
    ++m_totalPortScans;
    bool changed = false;

    // ---- 处理新增候选 ----
    QVector<QString> confirmedAdds;
    for (const QString& port : newDevices.keys()) {
        if (!m_currentDevices.contains(port)) {
            // 缓存待确认设备的详情
            if (!m_pendingAddDevices.contains(port)) {
                m_pendingAddDevices[port] = newDevices[port];
            }
            ++m_addedCandidateCount[port];
            if (m_addedCandidateCount[port] >= kDebounceThreshold) {
                const PortDeviceInfo& dev = m_pendingAddDevices[port];
                emit deviceAdded(dev);
                emit portAdded(port);  // 兼容旧接口
                confirmedAdds.append(port);
                m_addedCandidateCount.remove(port);
                m_pendingAddDevices.remove(port);
                ++m_totalArrivals;
                ++m_totalHotplugEvents;
                changed = true;
            }
        } else {
            m_addedCandidateCount.remove(port);
            m_pendingAddDevices.remove(port);
        }
    }

    // ---- 处理移除候选 ----
    QVector<QString> confirmedRemoves;
    for (const QString& port : m_currentDevices.keys()) {
        if (!newDevices.contains(port)) {
            ++m_removedCandidateCount[port];
            if (m_removedCandidateCount[port] >= kDebounceThreshold) {
                const PortDeviceInfo dev = m_currentDevices[port];
                emit deviceRemoved(dev);
                emit portRemoved(port);  // 兼容旧接口
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

    // ---- 清理过期候选 ----
    for (const QString& port : m_addedCandidateCount.keys()) {
        if (m_currentDevices.contains(port)) {
            m_addedCandidateCount.remove(port);
        }
    }

    // ---- 更新快照并发射总信号 ----
    if (changed) {
        ++m_totalChanges;
        for (const QString& port : confirmedAdds) {
            m_currentDevices[port] = newDevices[port];
        }
        for (const QString& port : confirmedRemoves) {
            m_currentDevices.remove(port);
        }

        // 记录事件日志
        int totalAfter = m_currentDevices.size();
        for (const QString& port : confirmedAdds) {
            PortEventRecord rec;
            rec.type = PortEventRecord::EventType::Arrival;
            rec.timestamp = QDateTime::currentDateTime();
            rec.device = m_currentDevices[port];
            rec.totalPortsAfterEvent = totalAfter;
            appendEventLog(rec);
        }
        for (const QString& port : confirmedRemoves) {
            PortEventRecord rec;
            rec.type = PortEventRecord::EventType::Removal;
            rec.timestamp = QDateTime::currentDateTime();
            rec.device = newDevices.contains(port)
                         ? newDevices[port]
                         : PortDeviceInfo{port};  // 尝试用最新信息，否则仅保留端口名
            rec.totalPortsAfterEvent = totalAfter;
            appendEventLog(rec);
        }

        m_lastChangeTimer.restart();
        m_hasLastChange = true;

        emit devicesChanged(currentDevices());
        emit portsChanged();  // 兼容旧接口
    }
}

// ---- 内部方法 ----

/**
 * @brief 查询系统中所有可用串口设备信息
 *
 * 使用 QSerialPortInfo 枚举所有串口，转换为 PortDeviceInfo。
 * @return 设备信息映射(端口名→信息)，按键排序
 */
QMap<QString, PortDeviceInfo> PortWatcher::queryAvailableDevices()
{
    QMap<QString, PortDeviceInfo> devices;
    try {
        const auto ports = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo& info : ports) {
            PortDeviceInfo dev = fromQtInfo(info);
            devices[dev.portName] = dev;
        }
    } catch (...) {
        ++m_totalScanErrors;
    }
    return devices;
}

/**
 * @brief 将QSerialPortInfo转换为PortDeviceInfo @param info Qt串口信息 @return 设备信息
 */
PortDeviceInfo PortWatcher::fromQtInfo(const QSerialPortInfo& info)
{
    PortDeviceInfo dev;
    dev.portName = info.portName();
    dev.description = info.description();
    dev.manufacturer = info.manufacturer();
    dev.serialNumber = info.serialNumber();
    dev.vendorId = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
    dev.productId = info.hasProductIdentifier() ? info.productIdentifier() : 0;
    if (dev.vendorId != 0)
        dev.vidHex = QString("%1").arg(dev.vendorId, 4, 16, QLatin1Char('0')).toUpper();
    if (dev.productId != 0)
        dev.pidHex = QString("%1").arg(dev.productId, 4, 16, QLatin1Char('0')).toUpper();
    return dev;
}

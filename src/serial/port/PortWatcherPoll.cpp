/**
 * @file PortWatcherPoll.cpp
 * @brief 串口热插拔检测器 - 定时轮询与设备查询实现
 *
 * 从 PortWatcher.cpp 拆分而来，包含定时轮询回调(带防抖)、
 * 系统设备查询和QSerialPortInfo转换方法。
 */

#include "serial/port/PortWatcher.h"
#include <QSerialPortInfo>

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

    /* ---- 处理新增候选 ---- */
    QVector<QString> confirmedAdds;
    for (const QString& port : newDevices.keys()) {
        if (!m_currentDevices.contains(port)) {
            if (!m_pendingAddDevices.contains(port)) {
                m_pendingAddDevices[port] = newDevices[port];
            }
            ++m_addedCandidateCount[port];
            if (m_addedCandidateCount[port] >= kDebounceThreshold) {
                const PortDeviceInfo& dev = m_pendingAddDevices[port];
                emit deviceAdded(dev);
                emit portAdded(port);
                confirmedAdds.append(port);
                m_addedCandidateCount.remove(port);
                m_pendingAddDevices.remove(port);
                ++m_totalArrivals;
                ++m_totalHotplugEvents;
                changed = true;
            }
        } else {
            if (m_addedCandidateCount.contains(port)) {
                ++m_totalDebounceSuppressions;
            }
            m_addedCandidateCount.remove(port);
            m_pendingAddDevices.remove(port);
        }
    }

    /* ---- 处理移除候选 ---- */
    QVector<QString> confirmedRemoves;
    for (const QString& port : m_currentDevices.keys()) {
        if (!newDevices.contains(port)) {
            ++m_removedCandidateCount[port];
            if (m_removedCandidateCount[port] >= kDebounceThreshold) {
                const PortDeviceInfo dev = m_currentDevices[port];
                emit deviceRemoved(dev);
                emit portRemoved(port);
                confirmedRemoves.append(port);
                m_removedCandidateCount.remove(port);
                ++m_totalRemovals;
                ++m_totalHotplugEvents;
                changed = true;
            }
        } else {
            if (m_removedCandidateCount.contains(port)) {
                ++m_totalDebounceSuppressions;
            }
            m_removedCandidateCount.remove(port);
        }
    }

    /* ---- 清理过期候选 ---- */
    for (const QString& port : m_addedCandidateCount.keys()) {
        if (m_currentDevices.contains(port)) {
            m_addedCandidateCount.remove(port);
        }
    }

    /* ---- 更新快照并发射总信号 ---- */
    if (changed) {
        ++m_totalChanges;
        for (const QString& port : confirmedAdds) {
            m_currentDevices[port] = newDevices[port];
        }
        for (const QString& port : confirmedRemoves) {
            m_currentDevices.remove(port);
        }

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
                         : PortDeviceInfo{port};
            rec.totalPortsAfterEvent = totalAfter;
            appendEventLog(rec);
        }

        m_lastChangeTimer.restart();
        m_hasLastChange = true;

        emit devicesChanged(currentDevices());
        emit portsChanged();
    }
}

/**
 * @brief 查询系统中所有可用串口设备信息
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

/** @brief 将QSerialPortInfo转换为PortDeviceInfo(含空端口名防护) @param info Qt串口信息 @return 设备信息 */
PortDeviceInfo PortWatcher::fromQtInfo(const QSerialPortInfo& info)
{
    PortDeviceInfo dev;
    dev.portName = info.portName().trimmed();
    // 空端口名回退: 使用系统路径或占位符，避免空键导致QMap异常
    if (dev.portName.isEmpty()) {
        dev.portName = info.systemLocation().isEmpty()
            ? QObject::tr("未知端口") : info.systemLocation();
    }
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

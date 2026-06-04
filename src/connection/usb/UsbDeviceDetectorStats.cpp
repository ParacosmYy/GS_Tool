/**
 * @file UsbDeviceDetectorStats.cpp
 * @brief USB设备检测器 — 统计查询与变更检测实现(拆分自UsbDeviceDetector.cpp)
 *
 * 包含设备查询、统计计数器访问、设备变更检测等辅助方法。
 * 核心扫描逻辑(构造/析构/扫描/监控启动停止)保留在UsbDeviceDetector.cpp。
 */

#include "connection/usb/UsbDeviceDetector.h"

/** @brief 获取指定VID/PID的USB设备详细信息 @param vid 厂商ID @param pid 产品ID @return 设备详情映射表 */
QVariantMap UsbDeviceDetector::deviceDetails(quint16 vid,
                                              quint16 pid) const {
    for (const QVariant& var : m_devices) {
        QVariantMap dev = var.toMap();
        if (dev["vid"].toUInt() == vid && dev["pid"].toUInt() == pid) {
            return dev;
        }
    }
    QVariantMap details;
    details["vid"] = vid;
    details["pid"] = pid;
    return details;
}

/** @brief 检查libusb是否可用 @return libusb已加载返回true */
bool UsbDeviceDetector::isLibusbAvailable() const {
    return m_libusbAvailable;
}

/** @brief 对比新旧设备列表，检测插入和移除事件 @param newList 最新扫描到的设备列表 */
void UsbDeviceDetector::detectChanges(const QVariantList& newList) {
    /* 检测插入的设备 */
    for (const QVariant& var : newList) {
        QVariantMap dev = var.toMap();
        bool found = false;
        for (const QVariant& oldVar : m_devices) {
            if (oldVar.toMap()["vid"] == dev["vid"] &&
                oldVar.toMap()["pid"] == dev["pid"]) {
                found = true;
                break;
            }
        }
        if (!found) {
            ++m_totalAttachEvents;
            emit deviceInserted(dev);
        }
    }

    /* 检测移除的设备 */
    for (const QVariant& var : m_devices) {
        QVariantMap dev = var.toMap();
        bool found = false;
        for (const QVariant& newVar : newList) {
            if (newVar.toMap()["vid"] == dev["vid"] &&
                newVar.toMap()["pid"] == dev["pid"]) {
                found = true;
                break;
            }
        }
        if (!found) {
            ++m_totalDetachEvents;
            emit deviceRemoved(dev);
        }
    }

    m_devices = newList;
}

/** @brief 获取累计检测周期次数 */
quint64 UsbDeviceDetector::totalDetectionCycles() const
{
    return m_totalDetectionCycles;
}

/** @brief 获取累计检测到设备次数 */
quint64 UsbDeviceDetector::totalDevicesDetected() const
{
    return m_totalDevicesDetected;
}

/** @brief 获取累计设备插入事件次数 */
quint64 UsbDeviceDetector::totalAttachEvents() const
{
    return m_totalAttachEvents;
}

/** @brief 获取累计设备拔出事件次数 */
quint64 UsbDeviceDetector::totalDetachEvents() const
{
    return m_totalDetachEvents;
}

/** @brief 获取libusb扫描调用次数 */
quint64 UsbDeviceDetector::totalLibusbScans() const
{
    return m_totalLibusbScans;
}

/** @brief 获取WMIC扫描调用次数 */
quint64 UsbDeviceDetector::totalWmicScans() const
{
    return m_totalWmicScans;
}

/** @brief 重置所有统计计数器 */
void UsbDeviceDetector::resetStatistics()
{
    m_totalDetectionCycles = 0;
    m_totalDevicesDetected = 0;
    m_totalAttachEvents = 0;
    m_totalDetachEvents = 0;
    m_totalLibusbScans = 0;
    m_totalWmicScans = 0;
}

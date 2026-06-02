/**
 * @file UsbDeviceDetector.cpp
 * @brief USB设备检测器实现
 *
 * 通过定时轮询检测USB设备变化。TODO: 使用libusb_hotplug回调替代轮询。
 */
#include "connection/usb/UsbDeviceDetector.h"

UsbDeviceDetector::UsbDeviceDetector(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout,
            this, &UsbDeviceDetector::onPollTimeout);
}

QVariantList UsbDeviceDetector::scanDevices() {
    QVariantList devices;
    // TODO: libusb_get_device_list 遍历设备
    m_devices = devices;
    return devices;
}

QVariantMap UsbDeviceDetector::deviceDetails(quint16 vid,
                                              quint16 pid) const {
    QVariantMap details;
    details["vid"]    = vid;
    details["pid"]    = pid;

    for (const QVariant& var : m_devices) {
        QVariantMap dev = var.toMap();
        if (dev["vid"].toUInt() == vid && dev["pid"].toUInt() == pid) {
            details = dev;
            break;
        }
    }
    return details;
}

void UsbDeviceDetector::startMonitoring(int intervalMs) {
    m_devices = scanDevices();
    m_pollTimer->start(intervalMs);
}

void UsbDeviceDetector::stopMonitoring() {
    m_pollTimer->stop();
}

void UsbDeviceDetector::onPollTimeout() {
    QVariantList newDevices = scanDevices();
    detectChanges(newDevices);
}

void UsbDeviceDetector::detectChanges(const QVariantList& newList) {
    // 检测插入的设备
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
            emit deviceInserted(dev);
        }
    }

    // 检测移除的设备
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
            emit deviceRemoved(dev);
        }
    }

    m_devices = newList;
}

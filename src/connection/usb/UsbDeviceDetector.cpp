/**
 * @file UsbDeviceDetector.cpp
 * @brief USB设备检测器实现
 *
 * 通过Windows WMIC命令枚举USB设备，定时轮询检测设备变化。
 * TODO: 未来可替换为libusb_hotplug回调或Windows SetupAPI。
 */
#include "connection/usb/UsbDeviceDetector.h"

#include <QProcess>
#include <QRegularExpression>

UsbDeviceDetector::UsbDeviceDetector(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout,
            this, &UsbDeviceDetector::onPollTimeout);
}

/**
 * @brief 扫描当前所有USB设备 — 通过WMIC枚举
 * 解析Win32_USBControllerDevice获取设备VID/PID和描述
 */
QVariantList UsbDeviceDetector::scanDevices() {
    QVariantList devices;

    QProcess process;
    process.start("wmic", {"path", "Win32_USBControllerDevice",
                            "get", "Dependent", "/format:list"});
    process.waitForFinished(5000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());

    // 解析设备ID行，提取VID/PID
    QRegularExpression vidPidRe("VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4})");
    QRegularExpression descRe("Description=([^\r\n]+)");

    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QRegularExpressionMatch match = vidPidRe.match(line);
        if (!match.hasMatch()) { continue; }

        QString vidStr = match.captured(1).toUpper();
        QString pidStr = match.captured(2).toUpper();
        bool okV = false, okP = false;
        quint16 vid = static_cast<quint16>(vidStr.toUInt(&okV, 16));
        quint16 pid = static_cast<quint16>(pidStr.toUInt(&okP, 16));
        if (!okV || !okP) { continue; }

        // 检查是否已存在（去重）
        bool dup = false;
        for (const QVariant& var : devices) {
            QVariantMap d = var.toMap();
            if (d["vid"].toUInt() == vid && d["pid"].toUInt() == pid) {
                dup = true;
                break;
            }
        }
        if (dup) { continue; }

        QVariantMap device;
        device["vid"] = vid;
        device["pid"] = pid;
        device["vidHex"] = vidStr;
        device["pidHex"] = pidStr;
        device["name"] = tr("USB设备 VID_%1 PID_%2")
                             .arg(vidStr, pidStr);
        device["manufacturer"] = QString();
        device["serial"] = QString();
        devices.append(device);
    }

    // 尝试获取更详细的信息
    QProcess detailProc;
    detailProc.start("wmic", {"path", "Win32_PnPEntity", "get",
                               "DeviceID,Name,Manufacturer",
                               "/format:list"});
    detailProc.waitForFinished(5000);
    QString detailOutput = QString::fromLocal8Bit(
        detailProc.readAllStandardOutput());

    // 匹配设备名和制造商
    for (int i = 0; i < devices.size(); ++i) {
        QVariantMap dev = devices[i].toMap();
        QString vidHex = dev["vidHex"].toString();
        QString pidHex = dev["pidHex"].toString();

        QRegularExpression nameRe(
            QString("VID_%1.*PID_%2.*\\nName=([^\r\n]+)")
                .arg(vidHex, pidHex),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch nameMatch = nameRe.match(detailOutput);
        if (nameMatch.hasMatch()) {
            dev["name"] = nameMatch.captured(1).trimmed();
        }

        QRegularExpression mfgRe(
            QString("VID_%1.*PID_%2.*\\nManufacturer=([^\r\n]+)")
                .arg(vidHex, pidHex),
            QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch mfgMatch = mfgRe.match(detailOutput);
        if (mfgMatch.hasMatch()) {
            dev["manufacturer"] = mfgMatch.captured(1).trimmed();
        }

        devices[i] = dev;
    }

    m_devices = devices;
    m_totalDevicesDetected += static_cast<quint64>(devices.size());
    return devices;
}

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

void UsbDeviceDetector::startMonitoring(int intervalMs) {
    m_devices = scanDevices();
    m_pollTimer->start(intervalMs);
}

void UsbDeviceDetector::stopMonitoring() {
    m_pollTimer->stop();
}

void UsbDeviceDetector::onPollTimeout() {
    ++m_totalDetectionCycles;
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
            ++m_totalAttachEvents;
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

/** @brief 重置所有统计计数器 */
void UsbDeviceDetector::resetStatistics()
{
    m_totalDetectionCycles = 0;
    m_totalDevicesDetected = 0;
    m_totalAttachEvents = 0;
    m_totalDetachEvents = 0;
}

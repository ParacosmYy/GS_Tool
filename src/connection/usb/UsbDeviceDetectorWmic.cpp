/**
 * @file UsbDeviceDetectorWmic.cpp
 * @brief USB设备检测器 — WMIC扫描实现(拆分自UsbDeviceDetector.cpp)
 *
 * 通过Windows WMIC命令枚举USB设备，不依赖libusb。
 * 解析Win32_USBControllerDevice获取设备VID/PID，
 * 再查询Win32_PnPEntity获取设备名和制造商。
 * 当libusb不可用时作为回退方案使用。
 */

#include "connection/usb/UsbDeviceDetector.h"

#include <QProcess>
#include <QRegularExpression>

/**
 * @brief 使用WMIC命令枚举USB设备(不依赖libusb)
 * 解析Win32_USBControllerDevice获取设备VID/PID，
 * 再查询Win32_PnPEntity获取设备名和制造商
 * @return 设备信息列表
 */
QVariantList UsbDeviceDetector::scanDevicesViaWmic() {
    QVariantList devices;
    ++m_totalWmicScans;

    QProcess process;
    process.start("wmic", {"path", "Win32_USBControllerDevice",
                            "get", "Dependent", "/format:list"});
    process.waitForFinished(5000);
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());

    /* 解析设备ID行，提取VID/PID */
    QRegularExpression vidPidRe("VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4})");

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

        /* 去重: 同一个VID/PID只保留一个 */
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
        device["bcdUSB"] = 0;
        device["deviceClass"] = 0;
        devices.append(device);
    }

    /* 尝试获取更详细的名称和制造商 */
    QProcess detailProc;
    detailProc.start("wmic", {"path", "Win32_PnPEntity", "get",
                               "DeviceID,Name,Manufacturer",
                               "/format:list"});
    detailProc.waitForFinished(5000);
    QString detailOutput = QString::fromLocal8Bit(
        detailProc.readAllStandardOutput());

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

    return devices;
}

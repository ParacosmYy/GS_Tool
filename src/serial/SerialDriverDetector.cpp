/**
 * @file SerialDriverDetector.cpp
 * @brief 串口驱动检测实现 - 检测系统已安装的USB转串口驱动芯片
 *
 * 支持检测的芯片:
 *   - CH340/CH341 (江苏沁恒)
 *   - CP2102/CP210x (Silicon Labs)
 *   - FT232/FTDI (FTDI)
 *   - PL2303 (Prolific)
 *
 * 检测方式: 遍历系统可用串口，匹配VID/PID或设备描述符中的芯片标识
 */

#include "serial/SerialDriverDetector.h"

#include <QObject>
#include <QSerialPortInfo>
#include <QDebug>

// 已知串口适配器驱动关键词列表
// 涵盖常见的 USB 转串口芯片厂商和型号
const QStringList SerialDriverDetector::kKnownDrivers = {
    QStringLiteral("CH340"),
    QStringLiteral("CH910"),
    QStringLiteral("CP210"),
    QStringLiteral("FTDI"),
    QStringLiteral("FT232"),
    QStringLiteral("PL2303"),
    QStringLiteral("Silicon Labs"),
    QStringLiteral("Prolific"),
    QStringLiteral("WCH"),
};

QVector<DriverInfo> SerialDriverDetector::detectDrivers()
{
    QVector<DriverInfo> result;

    // 为每个已知驱动创建条目，默认未安装
    for (const QString& keyword : kKnownDrivers) {
        DriverInfo info;
        info.driverName = keyword;
        info.installed = false;
        info.description = QString();
        result.append(info);
    }

    // 遍历系统中所有可用的串口
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : ports) {
        const QString desc = portInfo.description();
        const QString mfr  = portInfo.manufacturer();

        // 将描述和厂商信息合并后进行关键词匹配
        const QString combined = (desc + QLatin1Char(' ') + mfr).toUpper();

        for (int i = 0; i < result.size(); ++i) {
            if (!result[i].installed && combined.contains(result[i].driverName.toUpper())) {
                result[i].installed = true;
                // 保存更详细的描述信息
                if (!desc.isEmpty()) {
                    result[i].description = desc;
                } else if (!mfr.isEmpty()) {
                    result[i].description = mfr;
                }
            }
        }
    }

    return result;
}

bool SerialDriverDetector::hasAnyDriverInstalled()
{
    const QVector<DriverInfo> drivers = detectDrivers();
    for (const DriverInfo& info : drivers) {
        if (info.installed) {
            return true;
        }
    }
    return false;
}

QString SerialDriverDetector::driverStatusSummary()
{
    const QVector<DriverInfo> drivers = detectDrivers();
    const auto ports = QSerialPortInfo::availablePorts();

    // 情况1: 没有检测到任何串口设备
    if (ports.isEmpty()) {
        return QObject::tr("未检测到串口设备。\n"
                           "请检查:\n"
                           "1. USB转串口适配器已连接\n"
                           "2. 已安装串口驱动 (CH340/CP2102/FT232/PL2303)\n"
                           "3. 设备已上电");
    }

    // 情况2: 有串口设备，但没有匹配到已知驱动关键词
    // 这种情况下串口可能仍然可以正常使用（如原生串口、或不在已知列表中的芯片）
    bool anyKnown = false;
    QStringList detectedList;
    for (const DriverInfo& info : drivers) {
        if (info.installed) {
            anyKnown = true;
            detectedList.append(
                QStringLiteral("%1 (%2)").arg(info.driverName, info.description)
            );
        }
    }

    if (anyKnown) {
        return QObject::tr("已检测到的串口驱动:\n%1\n\n"
                           "可用端口数: %2")
            .arg(detectedList.join(QLatin1Char('\n')),
                 QString::number(ports.size()));
    }

    // 有端口但没有匹配到已知驱动，可能是系统原生串口或未列入的芯片
    QStringList portNames;
    for (const QSerialPortInfo& portInfo : ports) {
        portNames.append(portInfo.portName());
    }

    return QObject::tr("检测到串口但未匹配已知USB转串口驱动。\n"
                       "可用端口: %1\n"
                       "端口仍可能正常使用（原生COM口或未识别的适配器）。")
        .arg(portNames.join(QStringLiteral(", ")));
}

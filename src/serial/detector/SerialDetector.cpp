/**
 * @file SerialDetector.cpp
 * @brief 串口检测器实现 - 增强版设备检测，含USB芯片识别和丰富设备信息
 *
 * 功能:
 *   1. 定时扫描系统可用串口，检测插入/移除事件
 *   2. 通过VID/PID自动识别USB转串口芯片型号(CH340/CP2102/FT232/PL2303等)
 *   3. 生成友好名称和设备详情摘要
 *   4. 支持多维度查询(VID/描述/端口名/制造商/驱动类型)
 */

#include "serial/detector/SerialDetector.h"
#include <QSerialPortInfo>
#include <QSet>
#include <algorithm>

// ---- 芯片识别/厂商数据库已拆分至 SerialDetectorChip.cpp ----

// ---- SerialPortInfo 方法 ----

/** @brief 生成设备详情摘要(端口名+描述+制造商+VID/PID+芯片+驱动类型) @return 多行摘要文本 */
QString SerialPortInfo::toSummary() const
{
    QStringList lines;
    lines << QObject::tr("端口: %1").arg(portName);
    if (!description.isEmpty())
        lines << QObject::tr("描述: %1").arg(description);
    if (!manufacturer.isEmpty())
        lines << QObject::tr("制造商: %1").arg(manufacturer);
    if (!vidHex.isEmpty())
        lines << QObject::tr("VID: %1").arg(vidHex);
    if (!pidHex.isEmpty())
        lines << QObject::tr("PID: %1").arg(pidHex);
    if (!chipModel.isEmpty() && chipModel != "Unknown")
        lines << QObject::tr("芯片: %1").arg(chipModel);
    if (!driverType.isEmpty() && driverType != "Unknown")
        lines << QObject::tr("驱动: %1").arg(driverType);
    if (!serialNumber.isEmpty())
        lines << QObject::tr("序列号: %1").arg(serialNumber);
    if (!systemLocation.isEmpty())
        lines << QObject::tr("系统路径: %1").arg(systemLocation);
    return lines.join("\n");
}

// ---- SerialDetector 构造/析构 ----

/** @brief 构造串口检测器，连接定时器超时信号到刷新槽 @param parent 父对象 */
SerialDetector::SerialDetector(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SerialDetector::refreshPorts);
}

/** @brief 析构函数，停止监控 */
SerialDetector::~SerialDetector()
{
    stopMonitoring();
}

// ---- 监控控制 ----

/** @brief 启动串口热插拔监控(最小间隔100ms) @param intervalMs 轮询间隔(毫秒) */
void SerialDetector::startMonitoring(int intervalMs)
{
    m_timer.setInterval(qMax(100, intervalMs));
    refreshPorts();
    m_timer.start();
    m_monitoring = true;
    emit monitoringChanged(true);
}

/** @brief 停止串口热插拔监控 */
void SerialDetector::stopMonitoring()
{
    m_timer.stop();
    if (m_monitoring) {
        m_monitoring = false;
        emit monitoringChanged(false);
    }
}

/** @brief 获取当前所有已知可用端口信息列表 @return SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::availablePorts() const
{
    return m_knownPorts.values();
}

/** @brief 获取所有已知端口名称列表 @return 端口名称列表 */
QStringList SerialDetector::portNames() const
{
    return m_knownPorts.keys();
}

/** @brief 查询是否正在监控 @return true=监控中 */
bool SerialDetector::isMonitoring() const { return m_monitoring; }

// ---- 查询接口/统计/resetStatistics见 SerialDetectorQuery.cpp ----

/** @brief 获取已知芯片厂商数据库条目数 @return 数据库大小 */
int SerialDetector::knownVendorCount()
{
    return kKnownVendors.size();
}

// ---- 内部方法 ----

/** @brief 刷新端口列表，检测插入/移除事件并发射对应信号。包含空端口名过滤 */
void SerialDetector::refreshPorts()
{
    ++m_totalScans;
    QMap<QString, SerialPortInfo> current;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &pi : ports) {
        SerialPortInfo info = fromQtInfo(pi);
        /* 跳过无效端口: 端口名为空(如某些虚拟总线设备) */
        if (info.portName.isEmpty()) {
            continue;
        }
        info.isAvailable = true;
        current[info.portName] = info;
    }

    // 检测插入事件
    for (const auto &name : current.keys()) {
        if (!m_knownPorts.contains(name)) {
            ++m_totalInsertions;
            ++m_totalPortChangesDetected;
            emit portInserted(current[name]);
        }
    }

    // 检测移除事件
    for (const auto &name : m_knownPorts.keys()) {
        if (!current.contains(name)) {
            ++m_totalRemovals;
            ++m_totalPortChangesDetected;
            emit portRemoved(m_knownPorts[name]);
        }
    }

    m_knownPorts = current;
    emit portsChanged(current.values());
}

/**
 * @brief 将QSerialPortInfo转换为增强版SerialPortInfo
 *
 * 自动填充: 友好名称、驱动类型、芯片型号、VID/PID十六进制字符串。
 * 包含端口名称空值检查和VID/PID边界验证。
 * @param info Qt串口信息对象 @return 增强版内部SerialPortInfo结构
 */
SerialPortInfo SerialDetector::fromQtInfo(const QSerialPortInfo &info) const
{
    SerialPortInfo spi;
    // 基础字段 — 空字符串防护
    spi.portName = info.portName().trimmed();
    if (spi.portName.isEmpty()) {
        /* 端口名为空时使用系统路径作为回退标识，避免空键插入m_knownPorts */
        spi.portName = info.systemLocation().isEmpty()
            ? tr("未知端口") : info.systemLocation();
    }
    spi.description = info.description();
    spi.manufacturer = info.manufacturer();
    spi.serialNumber = info.serialNumber();
    spi.systemLocation = info.systemLocation();

    // VID/PID安全提取，确保hasVendorIdentifier/hasProductIdentifier前置检查
    spi.vendorId = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
    spi.productId = info.hasProductIdentifier() ? info.productIdentifier() : 0;

    // VID/PID边界验证: 确保不超过quint16范围(0x0000~0xFFFF)
    spi.vendorId = spi.vendorId & 0xFFFF;
    spi.productId = spi.productId & 0xFFFF;

    // 增强字段: VID/PID十六进制
    if (spi.vendorId != 0)
        spi.vidHex = QString("%1").arg(spi.vendorId, 4, 16, QLatin1Char('0')).toUpper();
    if (spi.productId != 0)
        spi.pidHex = QString("%1").arg(spi.productId, 4, 16, QLatin1Char('0')).toUpper();

    // 芯片识别 — null-safe: identifyChip内部使用QHash::value，VID/PID为0时返回"Unknown"
    spi.chipModel = identifyChip(spi.vendorId, spi.productId);
    spi.driverType = identifyDriverType(spi.description, spi.manufacturer);

    // 友好名称
    spi.friendlyName = generateFriendlyName(spi);

    return spi;
}

/**
 * @brief 自动生成友好名称
 *
 * 格式优先级: 芯片型号+端口名 > 描述+端口名 > 端口名
 * 示例: "CH340G (COM3)", "USB Serial (COM5)", "COM3"
 * @param info 端口信息 @return 友好名称字符串
 */
QString SerialDetector::generateFriendlyName(const SerialPortInfo &info)
{
    if (info.chipModel != "Unknown" && !info.chipModel.isEmpty()) {
        return QString("%1 (%2)").arg(info.chipModel, info.portName);
    }
    if (!info.description.isEmpty()) {
        return QString("%1 (%2)").arg(info.description, info.portName);
    }
    return info.portName;
}

// ---- 芯片识别/厂商数据库已拆分至 SerialDetectorChip.cpp ----

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

// ---- 已知USB转串口芯片数据库 ----

/**
 * @brief 已知USB转串口芯片厂商数据库
 *
 * 涵盖市面上绝大多数USB转串口芯片:
 * - Winchiphead (CH340/CH341): 最常见的国产芯片
 * - Silicon Labs (CP210x): 工业级常用
 * - FTDI (FT232/FT2232): 高性能工业级
 * - Prolific (PL2303): 早期通用芯片
 * - Microchip (MCP2200): 兼容性好的方案
 * - QinHeng (CH910/CH914): 新一代国产芯片
 * - STMicroelectronics (ST-LINK/V2): STM32调试器虚拟串口
 * - Espressif (ESP32): ESP32内置USB CDC
 */
const QVector<UsbVendorEntry> SerialDetector::kKnownVendors = {
    { 0x1A86, "Winchiphead",  "CH340"   },
    { 0x1A86, "Winchiphead",  "CH341"   },
    { 0x10C4, "Silicon Labs", "CP2102"  },
    { 0x10C4, "Silicon Labs", "CP2104"  },
    { 0x10C4, "Silicon Labs", "CP2108"  },
    { 0x10C4, "Silicon Labs", "CP2109"  },
    { 0x0403, "FTDI",         "FT232"   },
    { 0x0403, "FTDI",         "FT2232"  },
    { 0x0403, "FTDI",         "FT4232"  },
    { 0x0403, "FTDI",         "FT230X"  },
    { 0x067B, "Prolific",     "PL2303"  },
    { 0x04D8, "Microchip",    "MCP2200" },
    { 0x4348, "QinHeng",      "CH910"   },
    { 0x4348, "QinHeng",      "CH914"   },
    { 0x0483, "STMicroelectronics", "ST-LINK" },
    { 0x303A, "Espressif",    "ESP32"   },
    { 0x2E8A, "Raspberry Pi", "RP2040"  },
    { 0x16C0, "VOTI",         "Teensy"  },
    { 0x2341, "Arduino",      "UNO"     },
    { 0x2341, "Arduino",      "Mega"    },
    { 0x2886, "Seeed",        "XIAO"    },
};

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

// ---- 查询接口 ----

/** @brief 按USB厂商ID(VID)查找端口 @param vid USB厂商ID @return 匹配的SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::findByVendorId(quint16 vid) const
{
    ++m_totalVidLookups;
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.vendorId == vid) result.append(info);
    }
    return result;
}

/** @brief 按设备描述关键词查找端口(大小写不敏感) @param keyword 搜索关键词 @return 匹配的SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::findByDescription(const QString &keyword) const
{
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.description.contains(keyword, Qt::CaseInsensitive)) result.append(info);
    }
    return result;
}

/** @brief 按端口名称精确查找端口信息 @param name 端口名称(如"COM3") @return 匹配的SerialPortInfo，未找到返回空对象 */
SerialPortInfo SerialDetector::findByPortName(const QString &name) const
{
    return m_knownPorts.value(name);
}

/** @brief 按制造商关键词查找端口(大小写不敏感) @param keyword 制造商关键词 @return 匹配的SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::findByManufacturer(const QString &keyword) const
{
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.manufacturer.contains(keyword, Qt::CaseInsensitive)) result.append(info);
    }
    return result;
}

/** @brief 按驱动类型查找端口(精确匹配driverType字段) @param driverType 驱动类型 @return 匹配的SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::findByDriverType(const QString &driverType) const
{
    QList<SerialPortInfo> result;
    for (const auto &info : m_knownPorts) {
        if (info.driverType == driverType) result.append(info);
    }
    return result;
}

// ---- 芯片识别 ----

/**
 * @brief 根据VID查找已知芯片厂商信息 @param vid USB厂商ID @return 厂商信息
 *
 * 遍历已知芯片数据库，匹配VID返回对应的厂商名称和常见芯片型号。
 * 多个条目匹配同一VID时返回第一个(最常见芯片)。
 */
UsbVendorEntry SerialDetector::lookupVendor(quint16 vid)
{
    for (const auto &entry : kKnownVendors) {
        if (entry.vid == vid) return entry;
    }
    return { vid, QString(), QString() };
}

/**
 * @brief 根据VID/PID组合识别芯片型号 @param vid USB厂商ID @param pid USB产品ID @return 芯片型号
 *
 * 精确匹配常见芯片的VID/PID组合:
 * - CH340G: VID=1A86, PID=7523
 * - CH341:  VID=1A86, PID=5523
 * - CP2102: VID=10C4, PID=EA60
 * - CP2104: VID=10C4, PID=EA70
 * - FT232R: VID=0403, PID=6001
 * - FT2232: VID=0403, PID=6010
 * - PL2303: VID=067B, PID=2303
 * - MCP2200: VID=04D8, PID=00DF
 */
QString SerialDetector::identifyChip(quint16 vid, quint16 pid)
{
    // 精确VID/PID匹配表
    static const QHash<quint32, QString> chipMap = {
        // CH340/CH341系列 (Winchiphead)
        { (0x1A86u << 16) | 0x7523u, "CH340G"  },
        { (0x1A86u << 16) | 0x5523u, "CH341"   },
        { (0x1A86u << 16) | 0x1A00u, "CH9102F" },
        // Silicon Labs CP210x系列
        { (0x10C4u << 16) | 0xEA60u, "CP2102"  },
        { (0x10C4u << 16) | 0xEA70u, "CP2104"  },
        { (0x10C4u << 16) | 0xEA71u, "CP2109"  },
        { (0x10C4u << 16) | 0x80CAu, "CP2108"  },
        { (0x10C4u << 16) | 0x8A2Au, "CP2102N" },
        // FTDI系列
        { (0x0403u << 16) | 0x6001u, "FT232R"  },
        { (0x0403u << 16) | 0x6010u, "FT2232H" },
        { (0x0403u << 16) | 0x6011u, "FT4232H" },
        { (0x0403u << 16) | 0x6014u, "FT232H"  },
        { (0x0403u << 16) | 0x6015u, "FT230X"  },
        // Prolific系列
        { (0x067Bu << 16) | 0x2303u, "PL2303H" },
        { (0x067Bu << 16) | 0x23A3u, "PL2303TA"},
        { (0x067Bu << 16) | 0x23B3u, "PL2303TB"},
        // Microchip
        { (0x04D8u << 16) | 0x00DFu, "MCP2200" },
        // ST-LINK
        { (0x0483u << 16) | 0x374Bu, "ST-LINK/V2" },
        { (0x0483u << 16) | 0x3748u, "ST-LINK/V2-1"},
        { (0x0483u << 16) | 0x374Fu, "ST-LINK/V3" },
        // Espressif
        { (0x303Au << 16) | 0x1001u, "ESP32-S2" },
        { (0x303Au << 16) | 0x0002u, "ESP32-S3" },
        // Raspberry Pi
        { (0x2E8Au << 16) | 0x000Au, "RP2040"  },
        // Arduino
        { (0x2341u << 16) | 0x0043u, "UNO R3"  },
        { (0x2341u << 16) | 0x0010u, "Mega2560"},
        { (0x2341u << 16) | 0x804Eu, "Nano 33" },
    };
    quint32 key = (static_cast<quint32>(vid) << 16) | pid;
    return chipMap.value(key, "Unknown");
}

// ---- 统计 ----

/** @brief 获取累计检测到的不同VID数量 @return 不同VID计数 */
int SerialDetector::uniqueVidCount() const
{
    QSet<quint16> vids;
    for (const auto &info : m_knownPorts) {
        if (info.vendorId != 0) vids.insert(info.vendorId);
    }
    return vids.size();
}

/** @brief 获取已知芯片厂商数据库条目数 @return 数据库大小 */
int SerialDetector::knownVendorCount()
{
    return kKnownVendors.size();
}

/** @brief 重置所有统计计数器 */
void SerialDetector::resetStatistics()
{
    m_totalScans = 0;
    m_totalInsertions = 0;
    m_totalRemovals = 0;
    m_totalVidLookups = 0;
}

// ---- 内部方法 ----

/** @brief 刷新端口列表，检测插入/移除事件并发射对应信号 */
void SerialDetector::refreshPorts()
{
    ++m_totalScans;
    QMap<QString, SerialPortInfo> current;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &pi : ports) {
        SerialPortInfo info = fromQtInfo(pi);
        info.isAvailable = true;
        current[info.portName] = info;
    }

    // 检测插入事件
    for (const auto &name : current.keys()) {
        if (!m_knownPorts.contains(name)) {
            ++m_totalInsertions;
            emit portInserted(current[name]);
        }
    }

    // 检测移除事件
    for (const auto &name : m_knownPorts.keys()) {
        if (!current.contains(name)) {
            ++m_totalRemovals;
            emit portRemoved(m_knownPorts[name]);
        }
    }

    m_knownPorts = current;
    emit portsChanged(current.values());
}

/**
 * @brief 将QSerialPortInfo转换为增强版SerialPortInfo
 *
 * 自动填充: 友好名称、驱动类型、芯片型号、VID/PID十六进制字符串
 * @param info Qt串口信息对象 @return 增强版内部SerialPortInfo结构
 */
SerialPortInfo SerialDetector::fromQtInfo(const QSerialPortInfo &info) const
{
    SerialPortInfo spi;
    // 基础字段
    spi.portName = info.portName();
    spi.description = info.description();
    spi.manufacturer = info.manufacturer();
    spi.serialNumber = info.serialNumber();
    spi.systemLocation = info.systemLocation();
    spi.vendorId = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
    spi.productId = info.hasProductIdentifier() ? info.productIdentifier() : 0;

    // 增强字段: VID/PID十六进制
    if (spi.vendorId != 0)
        spi.vidHex = QString("%1").arg(spi.vendorId, 4, 16, QLatin1Char('0')).toUpper();
    if (spi.productId != 0)
        spi.pidHex = QString("%1").arg(spi.productId, 4, 16, QLatin1Char('0')).toUpper();

    // 芯片识别
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

/**
 * @brief 识别驱动类型(CH340/CP2102/FT232/PL2303等)
 *
 * 通过设备描述和制造商字段中的关键词匹配已知驱动芯片。
 * @param description 设备描述 @param manufacturer 制造商 @return 驱动类型字符串
 */
QString SerialDetector::identifyDriverType(const QString &description, const QString &manufacturer)
{
    // 合并描述和制造商进行关键词匹配
    QString combined = (description + " " + manufacturer).toUpper();

    if (combined.contains("CH340") || combined.contains("CH341"))
        return "CH340";
    if (combined.contains("CH910") || combined.contains("CH914"))
        return "CH910";
    if (combined.contains("CP210"))
        return "CP2102";
    if (combined.contains("FT232") || combined.contains("FT2232") || combined.contains("FTDI"))
        return "FT232";
    if (combined.contains("PL2303") || combined.contains("PROLIFIC"))
        return "PL2303";
    if (combined.contains("MCP2200") || combined.contains("MICROCHIP"))
        return "MCP2200";
    if (combined.contains("ST-LINK") || combined.contains("STLINK"))
        return "ST-LINK";
    if (combined.contains("ESP32") || combined.contains("ESPRESSIF"))
        return "ESP32";
    if (combined.contains("RP2040") || combined.contains("RASPBERRY"))
        return "RP2040";

    return "Unknown";
}

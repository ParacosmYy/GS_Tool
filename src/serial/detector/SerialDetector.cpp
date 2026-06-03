/**
 * @file SerialDetector.cpp
 * @brief 串口检测器实现 — 基于定时轮询的串口热插拔检测
 *
 * 定时扫描系统可用串口，检测插入/移除事件并发射信号通知。
 * 支持按VID、描述和端口名称查询。
 */

#include "serial/detector/SerialDetector.h"
#include <QSerialPortInfo>
#include <algorithm>

/** @brief 构造串口检测器，连接定时器超时信号到刷新槽 @param parent 父对象 */
SerialDetector::SerialDetector(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SerialDetector::refreshPorts);
}

/** @brief 析构函数，停止定时器 */
SerialDetector::~SerialDetector() = default;

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
    m_monitoring = false;
    emit monitoringChanged(false);
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

/** @brief 按USB厂商ID(VID)查找端口 @param vid USB厂商ID @return 匹配的SerialPortInfo列表 */
QList<SerialPortInfo> SerialDetector::findByVendorId(quint16 vid) const
{
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

/** @brief 刷新端口列表，检测插入/移除事件并发射对应信号 */
void SerialDetector::refreshPorts()
{
    QMap<QString, SerialPortInfo> current;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &pi : ports) {
        SerialPortInfo info = fromQtInfo(pi);
        info.isAvailable = true;
        current[info.portName] = info;
    }

    // Detect insertions
    for (const auto &name : current.keys()) {
        if (!m_knownPorts.contains(name)) {
            emit portInserted(current[name]);
        }
    }

    // Detect removals
    for (const auto &name : m_knownPorts.keys()) {
        if (!current.contains(name)) {
            emit portRemoved(m_knownPorts[name]);
        }
    }

    m_knownPorts = current;
    emit portsChanged(current.values());
}

/** @brief 将QSerialPortInfo转换为项目内部SerialPortInfo结构 @param info Qt串口信息对象 @return 内部SerialPortInfo结构 */
SerialPortInfo SerialDetector::fromQtInfo(const QSerialPortInfo &info) const
{
    SerialPortInfo spi;
    spi.portName = info.portName();
    spi.description = info.description();
    spi.manufacturer = info.manufacturer();
    spi.serialNumber = info.serialNumber();
    spi.systemLocation = info.systemLocation();
    spi.vendorId = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
    spi.productId = info.hasProductIdentifier() ? info.productIdentifier() : 0;
    return spi;
}

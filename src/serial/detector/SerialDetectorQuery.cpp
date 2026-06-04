/**
 * @file SerialDetectorQuery.cpp
 * @brief 串口检测器 - 多维度查询接口实现
 *
 * 从 SerialDetector.cpp 拆分而来，包含按VID/描述/端口名/
 * 制造商/驱动类型的查找方法和统计查询接口。
 */

#include "serial/detector/SerialDetector.h"
#include <QSet>

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

/** @brief 获取累计检测到的不同VID数量 @return 不同VID计数 */
int SerialDetector::uniqueVidCount() const
{
    QSet<quint16> vids;
    for (const auto &info : m_knownPorts) {
        if (info.vendorId != 0) vids.insert(info.vendorId);
    }
    return vids.size();
}

/** @brief 重置所有统计计数器 */
void SerialDetector::resetStatistics()
{
    m_totalScans = 0;
    m_totalInsertions = 0;
    m_totalRemovals = 0;
    m_totalVidLookups = 0;
    s_totalChipIdentifications = 0;
}

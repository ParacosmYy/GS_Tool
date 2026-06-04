/**
 * @file PacketBuilder.cpp
 * @brief 数据包构建器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 根据字段定义构建二进制数据包，支持多数据类型编码、CRC16校验、
 * SettingsManager模板持久化、字段校验和十六进制互转。
 */

#include "utils/packet/PacketBuilder.h"

#include <QDataStream>
#include <QFile>

/**
 * @brief 构造函数
 */
PacketBuilder::PacketBuilder(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 添加字段到列表
 */
void PacketBuilder::addField(const PacketField &field)
{
    ++m_totalFieldAdds;
    m_fields.append(field);
    emit fieldUpdated(m_fields.size() - 1);
}

/**
 * @brief 移除指定索引的字段
 */
void PacketBuilder::removeField(int index)
{
    if (index >= 0 && index < m_fields.size()) {
        m_fields.removeAt(index);
    }
}

/**
 * @brief 获取所有字段
 */
QList<PacketField> PacketBuilder::fields() const
{
    return m_fields;
}

/** @brief 获取累计构建次数 @return 构建总次数 */
quint64 PacketBuilder::totalBuilds() const { return m_totalBuilds; }
/** @brief 获取累计构建字节数 @return 字节总数 */
quint64 PacketBuilder::totalBytesBuilt() const { return m_totalBytesBuilt; }
/** @brief 获取累计模板加载次数 @return 加载总次数 */
quint64 PacketBuilder::totalTemplateLoads() const { return m_totalTemplateLoads; }
/** @brief 重置统计计数器(构建/字节/模板加载/字段添加/发送归零) */
void PacketBuilder::resetStats() { m_totalBuilds = 0; m_totalBytesBuilt = 0; m_totalTemplateLoads = 0; m_totalFieldAdds = 0; m_totalSends = 0; }

// 包构建/字段值编码/CRC校验见 PacketBuilderBuild.cpp
// 模板I/O/校验/Hex转换见 PacketBuilderTemplate.cpp

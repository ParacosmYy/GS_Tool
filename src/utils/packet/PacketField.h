/**
 * @file PacketField.h
 * @brief 数据包字段定义（仅头文件）
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 定义数据包中单个字段的结构：名称、偏移、大小、类型和值。
 */

#ifndef PACKETFIELD_H
#define PACKETFIELD_H

#include <QMetaType>
#include <QString>
#include <QVariant>

/**
 * @struct PacketField
 * @brief 数据包字段结构体，描述包中的一个字段
 */
struct PacketField
{
    QString name;           ///< 字段名称
    int offset = 0;         ///< 字节偏移量
    int size = 1;           ///< 字段大小（字节）
    QString dataType;       ///< 数据类型（如 "uint8"、"int16"、"float"）
    QVariant value;         ///< 字段值
};

// 注册元类型以便在信号槽中使用
Q_DECLARE_METATYPE(PacketField)

#endif // PACKETFIELD_H

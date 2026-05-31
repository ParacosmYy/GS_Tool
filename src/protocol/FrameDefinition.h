/**
 * @file FrameDefinition.h
 * @brief 帧格式定义 - 描述一帧数据的结构和字段布局
 *
 * 包含:
 *   - FieldDef: 单个数据字段的定义（名称/偏移/类型/缩放/单位）
 *   - ChecksumType: 校验算法枚举
 *   - FrameDefinition: 完整帧格式定义（帧头/帧尾/长度/校验/字段列表）
 *
 * 数据层: 不依赖任何表现层类，纯数据结构 + JSON序列化
 */

#ifndef FRAMEDEFINITION_H
#define FRAMEDEFINITION_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariantMap>
#include <QJsonObject>

/**
 * @brief 单个字段定义 - 描述帧内一个数据字段的属性
 *
 * 每个字段有独立的偏移、大小、数据类型、缩放系数和单位。
 * extractValue() 负责从 payload 中提取原始字节并转换为显示值。
 *
 * 边界保护:
 *   extractValue() 会检查 offset+size 是否超出 payload 范围，
 *   越界时返回无效 QVariant 并输出 qWarning 日志。
 */
struct FieldDef {
    QString name;           ///< 字段名称（如 "温度", "电压"）
    int offset = 0;         ///< 在帧payload内的字节偏移（必须 >= 0）
    int size = 1;           ///< 字段字节数（必须 > 0）
    double scale = 1.0;     ///< 缩放系数: 显示值 = 原始值 * scale + offsetVal
    double offsetVal = 0.0; ///< 偏移值
    QString unit;           ///< 单位（如 "mV", "°C"）

    /**
     * @brief 字段数据类型枚举
     *
     * 支持无符号整数、有符号整数、浮点数和原始字节。
     * LE = Little-Endian (小端), BE = Big-Endian (大端)
     */
    enum Type {
        UInt8,              ///< 无符号8位整数
        UInt16LE,           ///< 无符号16位整数（小端）
        UInt16BE,           ///< 无符号16位整数（大端）
        UInt32LE,           ///< 无符号32位整数（小端）
        UInt32BE,           ///< 无符号32位整数（大端）
        Int8,               ///< 有符号8位整数
        Int16LE,            ///< 有符号16位整数（小端）
        Int16BE,            ///< 有符号16位整数（大端）
        Float,              ///< IEEE 754 单精度浮点（小端）
        Raw                 ///< 原始字节（不做数值转换）
    };
    Type type = UInt8;      ///< 字段数据类型

    /**
     * @brief 从原始字节中提取并转换字段值
     * @param payload 帧有效数据区域（不含帧头/帧尾/校验）
     * @return 提取出的字段值（经过 scale + offsetVal 变换），越界时返回无效 QVariant
     *
     * 边界保护:
     *   - offset < 0: 返回无效 QVariant，输出 qWarning
     *   - size <= 0: 返回无效 QVariant，输出 qWarning
     *   - offset + size > payload.size(): 返回无效 QVariant，输出 qWarning
     */
    QVariant extractValue(const QByteArray& payload) const;

    /**
     * @brief 格式化显示值（带单位）
     * @param payload 帧有效数据区域
     * @return 格式化后的字符串，越界时返回 "N/A"
     */
    QString formatValue(const QByteArray& payload) const;

    /**
     * @brief 序列化为JSON对象
     * @return 包含字段所有属性的JSON对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象反序列化
     * @param obj JSON对象
     * @return 反序列化后的 FieldDef 实例
     */
    static FieldDef fromJson(const QJsonObject& obj);
};

/**
 * @brief 校验类型枚举
 *
 * 定义帧校验算法，FrameParser 根据此类型计算并验证校验值。
 */
enum class ChecksumType {
    None,           ///< 无校验
    Sum8,           ///< 8位累加和
    CRC8,           ///< CRC-8
    CRC16CCITT,     ///< CRC-16 CCITT
    CRC16Modbus,    ///< CRC-16 Modbus
    CRC32           ///< CRC-32
};

/**
 * @brief 完整帧格式定义 - 描述一帧数据的结构
 *
 * 一个完整的帧由以下部分组成:
 *   [帧头] [长度字段] [有效数据/Payload] [校验字段] [帧尾]
 *
 * 各部分都是可选的（最简帧可以只有帧头）。
 * fields 列表描述 payload 内各数据字段的布局。
 *
 * 设计模式: 值对象 - 纯数据结构，不可变语义（设置后通过 FrameParser 使用）
 */
struct FrameDefinition {
    QByteArray header;              ///< 帧头（如 AA 55），空=无帧头
    QByteArray footer;              ///< 帧尾（可选，如 0D 0A），空=无帧尾
    int lengthFieldOffset = -1;     ///< 长度字段在帧内的偏移（-1=无长度字段）
    int lengthFieldSize = 1;        ///< 长度字段字节数（1或2）
    bool lengthBigEndian = false;   ///< 长度字段大小端（false=小端）
    int lengthAdjust = 0;           ///< 长度字段值偏移（实际长度 = 字段值 + lengthAdjust）
    int checksumOffset = -1;        ///< 校验字段在帧内的偏移（-1=无校验）
    ChecksumType checksumType = ChecksumType::None; ///< 校验算法类型
    int checksumSize = 0;           ///< 校验字段字节数（1=Sum8/CRC8, 2=CRC16, 4=CRC32）
    int checksumStart = 0;          ///< 校验计算起始偏移（通常为0，从帧头开始）
    int checksumEnd = -1;           ///< 校验计算结束偏移（-1=到校验字段前）
    QVector<FieldDef> fields;       ///< 数据字段列表

    /**
     * @brief 计算整个帧的最大长度（用于缓冲区预分配）
     * @return 最小帧长度估算值，-1 表示无帧头（变长帧无法预知）
     *
     * 计算: header + lengthField + checksum + footer（不含payload）。
     * 注意: 返回的是固定部分的最小长度，不是完整帧的最大长度。
     */
    int maxFrameLength() const;

    /**
     * @brief 序列化为JSON对象
     * @return 包含帧定义所有属性的JSON对象
     *
     * header/footer 转为 HEX 字符串存储。
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象反序列化
     * @param obj JSON对象
     * @return 反序列化后的 FrameDefinition 实例
     */
    static FrameDefinition fromJson(const QJsonObject& obj);
};

#endif // FRAMEDEFINITION_H

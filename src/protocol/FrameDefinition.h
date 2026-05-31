#ifndef FRAMEDEFINITION_H
#define FRAMEDEFINITION_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QVariantMap>

// 单个字段定义 - 描述帧内一个数据字段的属性
struct FieldDef {
    QString name;           ///< 字段名称（如 "温度", "电压"）
    int offset = 0;         ///< 在帧payload内的字节偏移
    int size = 1;           ///< 字段字节数
    double scale = 1.0;     ///< 缩放系数: 显示值 = 原始值 × scale + offsetVal
    double offsetVal = 0.0; ///< 偏移值
    QString unit;           ///< 单位（如 "mV", "°C"）

    // 字段数据类型
    enum Type {
        UInt8,
        UInt16LE, UInt16BE,
        UInt32LE, UInt32BE,
        Int8,
        Int16LE, Int16BE,
        Float,
        Raw
    };
    Type type = UInt8;

    // 从原始字节中提取并转换字段值
    QVariant extractValue(const QByteArray& payload) const;

    // 格式化显示值（带单位）
    QString formatValue(const QByteArray& payload) const;

    // JSON序列化
    QJsonObject toJson() const;

    static FieldDef fromJson(const QJsonObject& obj);
};

// 校验类型枚举
enum class ChecksumType {
    None,
    Sum8,
    CRC8,
    CRC16CCITT,
    CRC16Modbus,
    CRC32
};

// 完整帧格式定义 - 描述一帧数据的结构
struct FrameDefinition {
    QByteArray header;          ///< 帧头（如 AA 55）
    QByteArray footer;          ///< 帧尾（可选，如 0D 0A）
    int lengthFieldOffset = -1; ///< 长度字段在帧内的偏移（-1=无长度字段）
    int lengthFieldSize = 1;    ///< 长度字段字节数（1或2）
    bool lengthBigEndian = false; ///< 长度字段大小端
    int lengthAdjust = 0;       ///< 长度字段值需要加的偏移（长度值=有效数据长度+lengthAdjust）
    int checksumOffset = -1;    ///< 校验字段在帧内的偏移（-1=无校验）
    ChecksumType checksumType = ChecksumType::None;
    int checksumSize = 0;       ///< 校验字段字节数
    int checksumStart = 0;      ///< 校验计算起始偏移（通常为0，从帧头开始）
    int checksumEnd = -1;       ///< 校验计算结束偏移（-1=到校验字段前）
    QVector<FieldDef> fields;   ///< 数据字段列表

    // 计算整个帧的最大长度（用于缓冲区预分配）
    // 如果没有长度字段，返回-1（变长帧无法预知）
    int maxFrameLength() const;

    // JSON序列化（保存/加载帧格式定义）
    QJsonObject toJson() const;

    static FrameDefinition fromJson(const QJsonObject& obj);
};

#endif // FRAMEDEFINITION_H

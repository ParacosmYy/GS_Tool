/**
 * @file IProtocolParser.h
 * @brief 协议解析器接口 - 自定义协议的统一解析协议
 *
 * 定义了协议解析的标准化接口，支持二进制协议的解析、序列化和校验。
 * 每种协议格式(自定义帧/Modbus/MQTT/CAN DBC)提供一个实现。
 *
 * 零出站依赖: 仅依赖 Qt Core 类型，不 include 任何项目头文件
 *
 * 设计模式:
 *   - 策略模式: 不同协议解析算法作为可互换的策略
 *   - 模板方法: 解析流程固定，子类实现具体的字段提取逻辑
 *
 * 协作关系:
 *   - ProtocolEngine: 持有多个 IProtocolParser，按协议类型分发解析
 *   - ProtocolView: 显示解析后的结构化数据
 *   - ProtocolSchema: 定义协议的元数据(字段/校验/字节序)
 */
#ifndef INTERFACES_IPROTOCOLPARSER_H
#define INTERFACES_IPROTOCOLPARSER_H

#include <QByteArray>
#include <QString>
#include <QVariant>
#include <QVector>

/**
 * @brief 协议字段解析结果
 */
struct ParsedField {
    QString name;       ///< 字段名称
    QVariant value;     ///< 字段值 (类型取决于协议定义)
    int bitOffset = 0;  ///< 字段在帧中的位偏移
    int bitWidth = 0;   ///< 字段位宽
    QString unit;       ///< 物理单位 (如 "V", "mA", "°C")
    QString description; ///< 字段说明
};

/**
 * @brief 协议解析器接口 - 自定义协议的统一解析协议
 *
 * 每种协议格式提供一个 IProtocolParser 实现，ProtocolEngine
 * 根据协议类型选择合适的解析器进行解析。
 */
class IProtocolParser {
public:
    virtual ~IProtocolParser() = default;

    /**
     * @brief 解析原始数据
     * @param raw 原始字节数据
     * @return 是否成功解析为有效帧
     */
    virtual bool parse(const QByteArray& raw) = 0;

    /**
     * @brief 将解析结果序列化为字节流
     * @return 序列化后的字节数据
     */
    virtual QByteArray serialize() const = 0;

    /**
     * @brief 校验当前帧的完整性
     * @return 校验是否通过
     */
    virtual bool validate() const = 0;

    /** @brief 获取协议名称 (如 "Modbus RTU", "自定义帧") */
    virtual QString protocolName() const = 0;

    /** @brief 获取解析后的字段列表 */
    virtual QVector<ParsedField> parsedFields() const = 0;

    /** @brief 获取解析错误信息 (当 parse() 返回 false 时) */
    virtual QString lastError() const = 0;
};

#endif // INTERFACES_IPROTOCOLPARSER_H

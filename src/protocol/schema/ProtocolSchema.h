/**
 * @file ProtocolSchema.h
 * @brief 自定义协议帧结构定义
 *
 * 描述一个串口协议的帧格式，包括帧定界规则、校验方式
 * 以及各字段的名称/偏移/大小/类型等元数据。
 */

#ifndef PROTOCOL_SCHEMA_H
#define PROTOCOL_SCHEMA_H

#include <QObject>
#include <QList>
#include <QString>
#include <QVector>
#include <QJsonObject>

/**
 * @class ProtocolSchema
 * @brief 协议帧结构描述
 *
 * 从 JSON 文件或字节数组加载帧定义，提供字段列表、帧定界规则与校验配置。
 */
class ProtocolSchema : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 校验算法类型
     */
    enum ChecksumType {
        None,        ///< 无校验
        Crc8,        ///< CRC-8
        Crc16Ccitt,  ///< CRC-16 CCITT
        Crc16Modbus, ///< CRC-16 Modbus
        Crc32,       ///< CRC-32
        Xor,         ///< 异或校验
        Sum          ///< 累加和校验
    };
    Q_ENUM(ChecksumType)

    /**
     * @struct FieldDefinition
     * @brief 单个字段描述
     */
    struct FieldDefinition {
        QString name;   ///< 字段名称
        int offset;     ///< 在帧中的字节偏移
        int size;       ///< 字段字节长度
        QString type;   ///< 数据类型（uint8/uint16/int32/float 等）
    };

    /**
     * @struct FramingRule
     * @brief 帧定界规则
     */
    struct FramingRule {
        QString type;            ///< 帧类型标识
        QVector<int> header;     ///< 帧头字节序列
        int lengthFieldOffset;   ///< 长度字段偏移（字节）
        int lengthFieldSize;     ///< 长度字段尺寸（字节）
        ChecksumType checksumType; ///< 校验算法
    };

    /** @brief 构造函数 */
    explicit ProtocolSchema(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolSchema() override;

    /**
     * @brief 从 JSON 文件加载协议定义
     * @param filePath JSON 文件路径
     * @return 加载成功返回 true
     */
    bool loadFromJson(const QString &filePath);

    /**
     * @brief 从 JSON 字节数据加载协议定义
     * @param jsonData JSON 格式的字节数组
     * @return 加载成功返回 true
     */
    bool loadFromJsonData(const QByteArray &jsonData);

    /**
     * @brief 获取协议名称
     * @return 协议名称字符串
     */
    QString name() const;

    /**
     * @brief 获取帧定界规则
     * @return 当前帧定界规则
     */
    FramingRule framing() const;

    /**
     * @brief 获取所有字段定义
     * @return 字段定义列表
     */
    QList<FieldDefinition> fields() const;

    /**
     * @brief 检查协议定义是否有效
     * @return 有效返回 true
     */
    bool isValid() const;

    /**
     * @brief 将当前协议定义序列化为 JSON 对象
     * @return 包含完整协议定义的 QJsonObject
     */
    QJsonObject toJson() const;

    /**
     * @brief 获取最近一次解析错误的描述信息
     * @return 错误描述字符串，无错误时为空
     */
    QString lastError() const;

    /* —— 可编程构造用 setter —— */

    /**
     * @brief 设置协议名称
     * @param name 协议名称
     */
    void setName(const QString &name);

    /**
     * @brief 设置帧定界规则
     * @param rule 帧定界规则
     */
    void setFraming(const FramingRule &rule);

    /**
     * @brief 追加一个字段定义
     * @param field 字段定义
     */
    void addField(const FieldDefinition &field);

    /**
     * @brief 设置协议定义是否有效
     * @param valid 有效标志
     */
    void setValid(bool valid);

private:
    QString m_name;                     ///< 协议名称
    FramingRule m_framing;              ///< 帧定界规则
    QList<FieldDefinition> m_fields;    ///< 字段定义列表
    bool m_valid = false;               ///< 协议是否有效
    QString m_lastError;                ///< 最近一次解析错误信息

    /**
     * @brief 将校验类型枚举转换为字符串
     * @param type 校验算法枚举值
     * @return 字符串标识
     */
    QString checksumTypeToString(ChecksumType type) const;

    /**
     * @brief 将字符串转换为校验类型枚举
     * @param str 校验算法字符串标识
     * @return 对应的枚举值
     */
    ChecksumType checksumTypeFromString(const QString &str) const;
};

#endif // PROTOCOL_SCHEMA_H

/** @file ProtocolSchema.h @brief 自定义协议帧结构定义 — 帧定界规则、校验方式、字段元数据 */
#ifndef PROTOCOL_SCHEMA_H
#define PROTOCOL_SCHEMA_H

#include <QObject>
#include <QList>
#include <QString>
#include <QVector>
#include <QJsonObject>

/** @brief 协议帧结构描述 — 从JSON文件/字节数组加载帧定义，提供字段列表、定界规则与校验配置 */
class ProtocolSchema : public QObject {
    Q_OBJECT

public:
    enum ChecksumType { None, Crc8, Crc16Ccitt, Crc16Modbus, Crc32, Xor, Sum }; ///< 校验算法类型
    Q_ENUM(ChecksumType)

    struct FieldDefinition { QString name; int offset; int size; QString type; }; ///< 单个字段描述
    struct FramingRule {                          ///< 帧定界规则
        QString type; QVector<int> header;
        int lengthFieldOffset, lengthFieldSize;
        ChecksumType checksumType;
    };

    explicit ProtocolSchema(QObject *parent = nullptr);
    ~ProtocolSchema() override;

    bool loadFromJson(const QString &filePath);     ///< 从JSON文件加载协议定义
    bool loadFromJsonData(const QByteArray &jsonData); ///< 从JSON字节数据加载
    QString name() const;                           ///< 获取协议名称
    FramingRule framing() const;                    ///< 获取帧定界规则
    QList<FieldDefinition> fields() const;          ///< 获取所有字段定义
    bool isValid() const;                           ///< 检查协议定义是否有效
    QJsonObject toJson() const;                     ///< 序列化为JSON对象
    QString lastError() const;                      ///< 最近解析错误描述

    void setName(const QString &name);              ///< 设置协议名称
    void setFraming(const FramingRule &rule);       ///< 设置帧定界规则
    void addField(const FieldDefinition &field);    ///< 追加字段定义
    void setValid(bool valid);                      ///< 设置有效标志

    // ---- 统计计数器接口 ----
    quint64 totalLoads() const;
    quint64 totalSaves() const;
    quint64 totalSchemas() const;
    quint64 totalFieldCount() const;
    quint64 maxSchemaSize() const;
    quint64 totalValidations() const;
    quint64 validationErrors() const;
    quint64 totalSchemaErrors() const;
    quint64 totalActiveSchemas() const;
    quint64 totalBuilds() const;
    void resetSchemaStatistics();
    void resetStats();

private:
    QString m_name;
    FramingRule m_framing;
    QList<FieldDefinition> m_fields;
    bool m_valid = false;
    QString m_lastError;
    mutable quint64 m_totalLoads = 0, m_totalSaves = 0, m_totalSchemas = 0;
    mutable quint64 m_totalFieldCount = 0, m_maxSchemaSize = 0, m_totalValidations = 0;
    mutable quint64 m_validationErrors = 0, m_totalSchemaErrors = 0, m_totalActiveSchemas = 0;
    mutable quint64 m_totalBuilds = 0;
    QString checksumTypeToString(ChecksumType type) const;
    ChecksumType checksumTypeFromString(const QString &str) const;
};

#endif // PROTOCOL_SCHEMA_H

/**
 * @file ProtocolSchema.cpp
 * @brief 自定义协议帧结构定义实现 — 核心加载与构造
 *
 * 从 JSON 文件或字节数组加载协议帧结构定义。
 * 字段访问、JSON 序列化、校验类型转换与统计计数器
 * 已拆分至 ProtocolSchemaFields.cpp。
 */

#include "protocol/schema/ProtocolSchema.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 构造函数 @param parent 父对象指针 */
ProtocolSchema::ProtocolSchema(QObject *parent)
    : QObject(parent)
    , m_valid(false)
{
}

/** @brief 析构函数 */
ProtocolSchema::~ProtocolSchema() = default;

/** @brief 从JSON文件加载协议定义 @param filePath JSON文件的完整路径 @return 加载成功返回true，否则返回false */
bool ProtocolSchema::loadFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_valid = false;
        m_lastError = tr("无法打开文件: %1").arg(filePath);
        ++m_totalSchemaErrors;
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        m_valid = false;
        m_lastError = tr("文件内容为空: %1").arg(filePath);
        ++m_totalSchemaErrors;
        return false;
    }

    return loadFromJsonData(data);
}

/** @brief 从JSON字节数据加载协议定义 @param jsonData JSON格式的字节数组 @return 解析成功返回true，否则返回false */
bool ProtocolSchema::loadFromJsonData(const QByteArray &jsonData)
{
    m_valid = false;
    m_lastError.clear();

    /* ---- 1. 解析 JSON 文档 ---- */
    ++m_totalValidations;
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        m_lastError = tr("JSON 解析失败: %1").arg(parseError.errorString());
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }
    if (!doc.isObject()) {
        m_lastError = tr("JSON 根元素必须是对象");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }

    const QJsonObject root = doc.object();

    /* ---- 2. 校验并读取协议名称 ---- */
    if (!root.contains(QStringLiteral("name"))) {
        m_lastError = tr("缺少必填字段: name");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }
    if (!root.value(QStringLiteral("name")).isString()) {
        m_lastError = tr("字段 name 必须为字符串类型");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }
    m_name = root.value(QStringLiteral("name")).toString();

    /* ---- 3. 校验并解析 framing 对象 ---- */
    if (!root.contains(QStringLiteral("framing"))) {
        m_lastError = tr("缺少必填字段: framing");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }
    if (!root.value(QStringLiteral("framing")).isObject()) {
        m_lastError = tr("字段 framing 必须为对象类型");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }

    const QJsonObject framingObj = root.value(QStringLiteral("framing")).toObject();

    /* 帧类型标识 */
    if (!framingObj.contains(QStringLiteral("type"))) {
        m_lastError = tr("framing 中缺少必填字段: type");
        ++m_validationErrors;
        ++m_totalSchemaErrors;
        return false;
    }
    m_framing.type = framingObj.value(QStringLiteral("type")).toString();

    /* 帧头字节序列 */
    if (framingObj.contains(QStringLiteral("header"))) {
        const QJsonArray headerArr = framingObj.value(QStringLiteral("header")).toArray();
        m_framing.header.clear();
        m_framing.header.reserve(headerArr.size());
        for (const QJsonValue &val : headerArr) {
            m_framing.header.append(val.toInt(0));
        }
    }

    /* 长度字段 offset / size */
    if (framingObj.contains(QStringLiteral("length_field"))
        && framingObj.value(QStringLiteral("length_field")).isObject()) {
        const QJsonObject lf = framingObj.value(QStringLiteral("length_field")).toObject();
        m_framing.lengthFieldOffset = lf.value(QStringLiteral("offset")).toInt(0);
        m_framing.lengthFieldSize = lf.value(QStringLiteral("size")).toInt(0);
    } else {
        m_framing.lengthFieldOffset = 0;
        m_framing.lengthFieldSize = 0;
    }

    /* 校验算法类型 */
    if (framingObj.contains(QStringLiteral("checksum"))
        && framingObj.value(QStringLiteral("checksum")).isObject()) {
        const QJsonObject chk = framingObj.value(QStringLiteral("checksum")).toObject();
        const QString chkType = chk.value(QStringLiteral("type")).toString();
        m_framing.checksumType = checksumTypeFromString(chkType);
    } else {
        m_framing.checksumType = ChecksumType::None;
    }

    /* ---- 4. 解析 fields 数组 ---- */
    m_fields.clear();
    if (root.contains(QStringLiteral("fields"))) {
        if (!root.value(QStringLiteral("fields")).isArray()) {
            m_lastError = tr("字段 fields 必须为数组类型");
            ++m_validationErrors;
            ++m_totalSchemaErrors;
            return false;
        }

        const QJsonArray fieldsArr = root.value(QStringLiteral("fields")).toArray();
        for (int i = 0; i < fieldsArr.size(); ++i) {
            if (!fieldsArr.at(i).isObject()) {
                m_lastError = tr("fields[%1] 必须为对象类型").arg(i);
                ++m_validationErrors;
                ++m_totalSchemaErrors;
                return false;
            }

            const QJsonObject fieldObj = fieldsArr.at(i).toObject();
            FieldDefinition field;
            field.name   = fieldObj.value(QStringLiteral("name")).toString();
            field.offset = fieldObj.value(QStringLiteral("offset")).toInt(0);
            field.size   = fieldObj.value(QStringLiteral("size")).toInt(0);
            field.type   = fieldObj.value(QStringLiteral("type")).toString();

            /* 校验必要字段 */
            if (field.name.isEmpty()) {
                m_lastError = tr("fields[%1] 缺少有效的 name 字段").arg(i);
                ++m_validationErrors;
                ++m_totalSchemaErrors;
                return false;
            }
            if (field.size <= 0) {
                m_lastError = tr("fields[%1] 的 size 必须大于 0").arg(i);
                ++m_validationErrors;
                ++m_totalSchemaErrors;
                return false;
            }

            m_fields.append(field);
        }
    }

    m_valid = true;

    /* 统计计数器更新 */
    ++m_totalSchemas;
    ++m_totalActiveSchemas;
    m_totalFieldCount += static_cast<quint64>(m_fields.size());
    quint64 schemaSize = static_cast<quint64>(jsonData.size());
    if (schemaSize > m_maxSchemaSize) {
        m_maxSchemaSize = schemaSize;
    }

    return true;
}

/* ──────────────── 可编程构造用 setter ──────────────── */

/** @brief 设置协议名称 @param name 协议名称 */
void ProtocolSchema::setName(const QString &name)
{
    m_name = name;
}

/** @brief 设置帧定界规则 @param rule 帧定界规则 */
void ProtocolSchema::setFraming(const FramingRule &rule)
{
    m_framing = rule;
}

/** @brief 追加一个字段定义 @param field 字段定义 */
void ProtocolSchema::addField(const FieldDefinition &field)
{
    m_fields.append(field);
}

/** @brief 设置协议定义是否有效 @param valid 有效标志 */
void ProtocolSchema::setValid(bool valid)
{
    m_valid = valid;
}

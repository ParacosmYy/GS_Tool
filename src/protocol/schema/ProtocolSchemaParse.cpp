/**
 * @file ProtocolSchemaParse.cpp
 * @brief 协议帧结构定义 - JSON字节数据解析实现
 *
 * 从 ProtocolSchema.cpp 拆分而来，包含 loadFromJsonData() 方法。
 * 该方法完成JSON文档解析→帧定界规则提取→字段数组解析→校验的完整流程。
 *
 * 从文件加载/构造/析构见 ProtocolSchema.cpp。
 */

#include "protocol/schema/ProtocolSchema.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 从JSON字节数据加载协议定义 @param jsonData JSON格式的字节数组 @return 解析成功返回true，否则返回false */
bool ProtocolSchema::loadFromJsonData(const QByteArray &jsonData)
{
    ++m_totalLoads;  // 累计加载计数(含成功和失败)
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

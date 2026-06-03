/**
 * @file ProtocolSchema.cpp
 * @brief 自定义协议帧结构定义实现
 *
 * 从 JSON 文件或字节数组加载协议帧结构定义，
 * 支持帧定界规则、校验配置与字段元数据的解析和序列化。
 */

#include "protocol/schema/ProtocolSchema.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ProtocolSchema::ProtocolSchema(QObject *parent)
    : QObject(parent)
    , m_valid(false)
{
}

/**
 * @brief 析构函数
 */
ProtocolSchema::~ProtocolSchema() = default;

/**
 * @brief 从 JSON 文件加载协议定义
 *
 * 读取指定路径的 JSON 文件并解析为协议帧结构定义。
 * 成功后 isValid() 返回 true，失败时可通过 lastError() 获取错误信息。
 *
 * @param filePath JSON 文件的完整路径
 * @return 加载并解析成功返回 true，否则返回 false
 */
bool ProtocolSchema::loadFromJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_valid = false;
        m_lastError = QStringLiteral("无法打开文件: %1").arg(filePath);
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        m_valid = false;
        m_lastError = QStringLiteral("文件内容为空: %1").arg(filePath);
        return false;
    }

    return loadFromJsonData(data);
}

/**
 * @brief 从 JSON 字节数据加载协议定义
 *
 * 将给定的 JSON 格式字节数组解析为协议帧结构定义。
 * 成功后 isValid() 返回 true。
 *
 * JSON 结构要求:
 * - "name": 协议名称字符串
 * - "framing": 帧定界规则对象
 *   - "type": 帧类型标识
 *   - "header": 帧头字节数组
 *   - "length_field": { "offset": int, "size": int }
 *   - "checksum": { "type": string }
 * - "fields": 字段定义数组，每项含 name/offset/size/type
 *
 * @param jsonData JSON 格式的字节数组
 * @return 解析成功返回 true，否则返回 false
 */
bool ProtocolSchema::loadFromJsonData(const QByteArray &jsonData)
{
    m_valid = false;
    m_lastError.clear();

    /* ---- 1. 解析 JSON 文档 ---- */
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (doc.isNull()) {
        m_lastError = QStringLiteral("JSON 解析失败: %1").arg(parseError.errorString());
        return false;
    }
    if (!doc.isObject()) {
        m_lastError = QStringLiteral("JSON 根元素必须是对象");
        return false;
    }

    const QJsonObject root = doc.object();

    /* ---- 2. 校验并读取协议名称 ---- */
    if (!root.contains(QStringLiteral("name"))) {
        m_lastError = QStringLiteral("缺少必填字段: name");
        return false;
    }
    if (!root.value(QStringLiteral("name")).isString()) {
        m_lastError = QStringLiteral("字段 name 必须为字符串类型");
        return false;
    }
    m_name = root.value(QStringLiteral("name")).toString();

    /* ---- 3. 校验并解析 framing 对象 ---- */
    if (!root.contains(QStringLiteral("framing"))) {
        m_lastError = QStringLiteral("缺少必填字段: framing");
        return false;
    }
    if (!root.value(QStringLiteral("framing")).isObject()) {
        m_lastError = QStringLiteral("字段 framing 必须为对象类型");
        return false;
    }

    const QJsonObject framingObj = root.value(QStringLiteral("framing")).toObject();

    /* 帧类型标识 */
    if (!framingObj.contains(QStringLiteral("type"))) {
        m_lastError = QStringLiteral("framing 中缺少必填字段: type");
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
            m_lastError = QStringLiteral("字段 fields 必须为数组类型");
            return false;
        }

        const QJsonArray fieldsArr = root.value(QStringLiteral("fields")).toArray();
        for (int i = 0; i < fieldsArr.size(); ++i) {
            if (!fieldsArr.at(i).isObject()) {
                m_lastError = QStringLiteral("fields[%1] 必须为对象类型").arg(i);
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
                m_lastError = QStringLiteral("fields[%1] 缺少有效的 name 字段").arg(i);
                return false;
            }
            if (field.size <= 0) {
                m_lastError = QStringLiteral("fields[%1] 的 size 必须大于 0").arg(i);
                return false;
            }

            m_fields.append(field);
        }
    }

    m_valid = true;

    /* 统计计数器更新 */
    ++m_totalSchemas;
    m_totalFieldCount += static_cast<quint64>(m_fields.size());
    quint64 schemaSize = static_cast<quint64>(jsonData.size());
    if (schemaSize > m_maxSchemaSize) {
        m_maxSchemaSize = schemaSize;
    }

    return true;
}

/**
 * @brief 将当前协议定义序列化为 JSON 对象
 *
 * 输出格式与 loadFromJsonData() 输入格式一致，
 * 可用于持久化存储或调试输出。
 *
 * @return 包含完整协议定义的 QJsonObject，未加载时返回空对象
 */
QJsonObject ProtocolSchema::toJson() const
{
    QJsonObject root;

    /* 协议名称 */
    root[QStringLiteral("name")] = m_name;

    /* 帧定界规则 */
    QJsonObject framingObj;
    framingObj[QStringLiteral("type")] = m_framing.type;

    /* 帧头字节序列 */
    QJsonArray headerArr;
    for (int byte : m_framing.header) {
        headerArr.append(byte);
    }
    framingObj[QStringLiteral("header")] = headerArr;

    /* 长度字段 */
    QJsonObject lfObj;
    lfObj[QStringLiteral("offset")] = m_framing.lengthFieldOffset;
    lfObj[QStringLiteral("size")]   = m_framing.lengthFieldSize;
    framingObj[QStringLiteral("length_field")] = lfObj;

    /* 校验配置 */
    QJsonObject chkObj;
    chkObj[QStringLiteral("type")] = checksumTypeToString(m_framing.checksumType);
    framingObj[QStringLiteral("checksum")] = chkObj;

    root[QStringLiteral("framing")] = framingObj;

    /* 字段定义数组 */
    QJsonArray fieldsArr;
    for (const FieldDefinition &field : m_fields) {
        QJsonObject fieldObj;
        fieldObj[QStringLiteral("name")]   = field.name;
        fieldObj[QStringLiteral("offset")] = field.offset;
        fieldObj[QStringLiteral("size")]   = field.size;
        fieldObj[QStringLiteral("type")]   = field.type;
        fieldsArr.append(fieldObj);
    }
    root[QStringLiteral("fields")] = fieldsArr;

    return root;
}

/**
 * @brief 获取协议名称
 * @return 协议名称字符串，未加载时为空
 */
QString ProtocolSchema::name() const
{
    return m_name;
}

/**
 * @brief 获取帧定界规则
 * @return 当前帧定界规则
 */
ProtocolSchema::FramingRule ProtocolSchema::framing() const
{
    return m_framing;
}

/**
 * @brief 获取所有字段定义列表
 * @return 字段定义列表，未加载时为空
 */
QList<ProtocolSchema::FieldDefinition> ProtocolSchema::fields() const
{
    return m_fields;
}

/**
 * @brief 检查当前协议定义是否有效
 * @return 协议定义有效返回 true，否则返回 false
 */
bool ProtocolSchema::isValid() const
{
    return m_valid;
}

/**
 * @brief 获取最近一次解析错误的描述信息
 * @return 错误描述字符串，无错误时为空
 */
QString ProtocolSchema::lastError() const
{
    return m_lastError;
}

/* ──────────────── 可编程构造用 setter ──────────────── */

/**
 * @brief 设置协议名称
 * @param name 协议名称
 */
void ProtocolSchema::setName(const QString &name)
{
    m_name = name;
}

/**
 * @brief 设置帧定界规则
 * @param rule 帧定界规则
 */
void ProtocolSchema::setFraming(const FramingRule &rule)
{
    m_framing = rule;
}

/**
 * @brief 追加一个字段定义
 * @param field 字段定义
 */
void ProtocolSchema::addField(const FieldDefinition &field)
{
    m_fields.append(field);
}

/**
 * @brief 设置协议定义是否有效
 * @param valid 有效标志
 */
void ProtocolSchema::setValid(bool valid)
{
    m_valid = valid;
}

/**
 * @brief 将校验类型枚举值转换为字符串标识
 * @param type 校验算法枚举值
 * @return 对应的字符串标识，未知类型返回 "none"
 */
QString ProtocolSchema::checksumTypeToString(ChecksumType type) const
{
    switch (type) {
    case ChecksumType::None:        return QStringLiteral("none");
    case ChecksumType::Crc8:        return QStringLiteral("crc8");
    case ChecksumType::Crc16Ccitt:  return QStringLiteral("crc16_ccitt");
    case ChecksumType::Crc16Modbus: return QStringLiteral("crc16_modbus");
    case ChecksumType::Crc32:       return QStringLiteral("crc32");
    case ChecksumType::Xor:         return QStringLiteral("xor");
    case ChecksumType::Sum:         return QStringLiteral("sum");
    default:                        return QStringLiteral("none");
    }
}

/**
 * @brief 将字符串标识转换为校验类型枚举值
 * @param str 校验算法字符串标识
 * @return 对应的枚举值，无法识别时返回 None
 */
ProtocolSchema::ChecksumType ProtocolSchema::checksumTypeFromString(const QString &str) const
{
    if (str == QStringLiteral("none"))        return ChecksumType::None;
    if (str == QStringLiteral("crc8"))        return ChecksumType::Crc8;
    if (str == QStringLiteral("crc16_ccitt")) return ChecksumType::Crc16Ccitt;
    if (str == QStringLiteral("crc16_modbus"))return ChecksumType::Crc16Modbus;
    if (str == QStringLiteral("crc32"))       return ChecksumType::Crc32;
    if (str == QStringLiteral("xor"))         return ChecksumType::Xor;
    if (str == QStringLiteral("sum"))         return ChecksumType::Sum;
    return ChecksumType::None;
}

// ============================================================================
// 统计计数器接口
// ============================================================================

/** @brief 获取已加载的协议定义总数 @return 累计加载次数 */
quint64 ProtocolSchema::totalSchemas() const
{
    return m_totalSchemas;
}

/** @brief 获取所有已加载协议的字段总数 @return 累计字段数 */
quint64 ProtocolSchema::totalFieldCount() const
{
    return m_totalFieldCount;
}

/** @brief 获取历史最大协议定义大小(字节) @return 最大JSON字节数 */
quint64 ProtocolSchema::maxSchemaSize() const
{
    return m_maxSchemaSize;
}

/** @brief 重置所有统计计数器(加载数/字段数/最大大小) */
void ProtocolSchema::resetStats()
{
    m_totalSchemas = 0;
    m_totalFieldCount = 0;
    m_maxSchemaSize = 0;
}

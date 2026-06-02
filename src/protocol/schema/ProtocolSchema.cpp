/**
 * @file ProtocolSchema.cpp
 * @brief 自定义协议帧结构定义实现
 */

#include "protocol/schema/ProtocolSchema.h"

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
 * 成功后 isValid() 返回 true。
 *
 * @param filePath JSON 文件的完整路径
 * @return 加载并解析成功返回 true，否则返回 false
 */
bool ProtocolSchema::loadFromJson(const QString &filePath)
{
    Q_UNUSED(filePath)
    // TODO: 读取 JSON 文件并解析帧结构
    return false;
}

/**
 * @brief 从 JSON 字节数据加载协议定义
 *
 * 将给定的 JSON 格式字节数组解析为协议帧结构定义。
 * 成功后 isValid() 返回 true。
 *
 * @param jsonData JSON 格式的字节数组
 * @return 解析成功返回 true，否则返回 false
 */
bool ProtocolSchema::loadFromJsonData(const QByteArray &jsonData)
{
    Q_UNUSED(jsonData)
    // TODO: 解析 JSON 数据填充 m_name / m_framing / m_fields
    return false;
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

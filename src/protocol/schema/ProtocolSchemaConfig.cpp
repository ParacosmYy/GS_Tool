/**
 * @file ProtocolSchemaConfig.cpp
 * @brief 自定义协议帧结构定义 — 可编程构造用 setter 实现
 *
 * 从 ProtocolSchema.cpp 拆分而来，包含 setName、setFraming、
 * addField、setValid 等编程式构建协议定义的配置方法。
 */

#include "protocol/schema/ProtocolSchema.h"

/** @brief 设置协议名称 @param name 协议名称 */
void ProtocolSchema::setName(const QString &name)
{
    m_name = name;
}

/** @brief 设置帧定界规则 @param rule 帧定界规则 */
void ProtocolSchema::setFraming(const FramingRule &rule)
{
    m_framing = rule;
    ++m_totalBuilds;  // 累计编程构造计数
}

/** @brief 追加一个字段定义 @param field 字段定义 */
void ProtocolSchema::addField(const FieldDefinition &field)
{
    m_fields.append(field);
    ++m_totalBuilds;  // 累计编程构造计数
}

/** @brief 设置协议定义是否有效 @param valid 有效标志 */
void ProtocolSchema::setValid(bool valid)
{
    m_valid = valid;
}

/**
 * @file TriggerEngine.cpp
 * @brief 触发器引擎实现 — 骨架文件
 */

#include "automation/TriggerEngine.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TriggerEngine::TriggerEngine(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
{
}

/**
 * @brief 评估原始字节数据
 *
 * 遍历所有已启用规则，对数据执行匹配。
 * ExactString/Regex 模式将 data 当作 UTF-8 字符串处理；
 * HexBytes 模式将 data 与模式的十六进制解码结果比较。
 *
 * @param data 待评估的原始数据
 */
void TriggerEngine::evaluateData(const QByteArray& data)
{
    Q_UNUSED(data)
    // TODO: 遍历 m_rules，对每条已启用规则执行匹配
}

/**
 * @brief 评估解析后的数值
 *
 * 仅匹配 ValueRange 类型的规则。
 *
 * @param name 数据标识
 * @param value 数值
 */
void TriggerEngine::evaluateValue(const QString& name, double value)
{
    Q_UNUSED(name)
    Q_UNUSED(value)
    // TODO: 遍历 m_rules，匹配 ValueRange 规则
}

/**
 * @brief 添加一条触发器规则
 * @param rule 规则配置
 */
void TriggerEngine::addRule(const TriggerRuleConfig& rule)
{
    Q_UNUSED(rule)
    // TODO: 将规则追加到 m_rules
}

/**
 * @brief 移除指定索引的规则
 * @param index 规则索引
 */
void TriggerEngine::removeRule(int index)
{
    Q_UNUSED(index)
    // TODO: 移除 m_rules[index]
}

/**
 * @brief 设置所有规则的启用/禁用状态
 * @param enabled true 启用，false 禁用
 */
void TriggerEngine::setRulesEnabled(bool enabled)
{
    m_enabled = enabled;
}

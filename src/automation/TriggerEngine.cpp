/**
 * @file TriggerEngine.cpp
 * @brief 触发器引擎实现 — 数据匹配评估核心逻辑
 */

#include "automation/TriggerEngine.h"

#include <QRegularExpression>
#include <QString>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TriggerEngine::TriggerEngine(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
    , m_matchCount(0)
{
}

/**
 * @brief 评估原始字节数据
 *
 * 遍历所有已启用规则，对数据执行匹配:
 *   - ExactString: 在 data 中搜索 pattern 的 UTF-8 编码
 *   - Regex: 使用 QRegularExpression 正则匹配
 *   - HexBytes: 将 pattern 解析为十六进制字节序列后搜索
 *   - ValueRange: 跳过（需解析后的数值，非原始数据）
 *
 * @param data 待评估的原始数据
 */
void TriggerEngine::evaluateData(const QByteArray& data)
{
    if (!m_enabled || data.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_rules.size(); ++i) {
        const TriggerRuleConfig& rule = m_rules.at(i);
        if (!rule.enabled) {
            continue;
        }

        bool matched = false;

        switch (rule.matchMode) {
        case MatchMode::ExactString: {
            /* 精确字符串匹配: data 包含 pattern 的 UTF-8 编码 */
            matched = data.contains(rule.pattern.toUtf8());
            break;
        }
        case MatchMode::Regex: {
            /* 正则表达式匹配 */
            QRegularExpression re(rule.pattern);
            if (re.isValid()) {
                QString text = QString::fromUtf8(data);
                matched = re.match(text).hasMatch();
            }
            break;
        }
        case MatchMode::HexBytes: {
            /* 十六进制字节序列匹配 */
            QByteArray hexBytes = QByteArray::fromHex(rule.pattern.toUtf8());
            if (!hexBytes.isEmpty()) {
                matched = data.contains(hexBytes);
            }
            break;
        }
        case MatchMode::ValueRange: {
            /* 数值范围匹配需要解析后的值，此处跳过 */
            break;
        }
        }

        if (matched) {
            ++m_matchCount;
            emit triggered(i, rule.name);
            emit actionRequired(static_cast<int>(rule.actionType), rule.actionData);
        }
    }
}

/**
 * @brief 评估解析后的数值
 *
 * 仅匹配 ValueRange 类型的规则，检查数值是否在 [valueMin, valueMax] 范围内。
 *
 * @param name 数据标识
 * @param value 数值
 */
void TriggerEngine::evaluateValue(const QString& name, double value)
{
    Q_UNUSED(name)

    if (!m_enabled) {
        return;
    }

    for (int i = 0; i < m_rules.size(); ++i) {
        const TriggerRuleConfig& rule = m_rules.at(i);
        if (!rule.enabled || rule.matchMode != MatchMode::ValueRange) {
            continue;
        }

        if (value >= rule.valueMin && value <= rule.valueMax) {
            ++m_matchCount;
            emit triggered(i, rule.name);
            emit actionRequired(static_cast<int>(rule.actionType), rule.actionData);
        }
    }
}

/**
 * @brief 添加一条触发器规则
 * @param rule 规则配置
 */
void TriggerEngine::addRule(const TriggerRuleConfig& rule)
{
    m_rules.append(rule);
}

/**
 * @brief 移除指定索引的规则
 * @param index 规则索引
 */
void TriggerEngine::removeRule(int index)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules.removeAt(index);
    }
}

/**
 * @brief 设置所有规则的启用/禁用状态
 * @param enabled true 启用，false 禁用
 */
void TriggerEngine::setRulesEnabled(bool enabled)
{
    m_enabled = enabled;
}

/**
 * @brief 设置指定规则的启用/禁用状态
 * @param index 规则索引
 * @param enabled true 启用，false 禁用
 */
void TriggerEngine::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules[index].enabled = enabled;
    }
}

/**
 * @brief 获取所有规则列表
 * @return 规则配置列表的常引用
 */
const QList<TriggerRuleConfig>& TriggerEngine::rules() const
{
    return m_rules;
}

/**
 * @brief 获取累计成功匹配次数
 * @return 匹配次数
 */
int TriggerEngine::matchCount() const
{
    return m_matchCount;
}

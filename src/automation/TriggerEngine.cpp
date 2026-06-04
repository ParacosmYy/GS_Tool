/**
 * @file TriggerEngine.cpp
 * @brief 触发器引擎实现 — 数据匹配评估核心逻辑
 */

#include "automation/TriggerEngine.h"

#include <QRegularExpression>
#include <QString>

/** @brief 构造函数 @param parent 父对象 */
TriggerEngine::TriggerEngine(QObject* parent)
    : QObject(parent)
    , m_enabled(true)
    , m_matchCount(0)
    , m_hasMatched(false)
{
}

/** @brief 评估原始字节数据，遍历已启用规则执行匹配(ExactString/Regex/HexBytes) @param data 待评估的原始数据 */
void TriggerEngine::evaluateData(const QByteArray& data)
{
    if (!m_enabled || data.isEmpty()) {
        return;
    }

    /* 每次调用计为一次评估 */
    ++m_totalEvaluations;

    for (int i = 0; i < m_rules.size(); ++i) {
        const TriggerRuleConfig& rule = m_rules.at(i);
        if (!rule.enabled) {
            ++m_totalTriggersDisabled;
            continue;
        }

        /* 每条启用规则计为一次评估 */
        ++m_totalTriggersEvaluated;

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
            } else {
                /* 正则编译失败，计入错误和动作错误 */
                ++m_totalErrors;
                ++m_totalActionErrors;
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
            ++m_totalMatches;
            ++m_totalActionsExecuted;
            ++m_totalTriggersFired;
            if (i < m_ruleMatchCounts.size()) {
                ++m_ruleMatchCounts[i];
            }
            m_lastMatchTimer.start();
            m_hasMatched = true;
            emit triggered(i, rule.name);
            emit actionRequired(static_cast<int>(rule.actionType), rule.actionData);
        }
    }
}

/** @brief 评估解析后的数值，仅匹配ValueRange规则检查数值范围 @param name 数据标识 @param value 数值 */
void TriggerEngine::evaluateValue(const QString& name, double value)
{
    Q_UNUSED(name)

    if (!m_enabled) {
        return;
    }

    /* 每次调用计为一次评估 */
    ++m_totalEvaluations;

    for (int i = 0; i < m_rules.size(); ++i) {
        const TriggerRuleConfig& rule = m_rules.at(i);
        if (!rule.enabled) {
            ++m_totalTriggersDisabled;
            continue;
        }
        if (rule.matchMode != MatchMode::ValueRange) {
            continue;
        }

        /* 每条启用的 ValueRange 规则计为一次评估 */
        ++m_totalTriggersEvaluated;

        if (value >= rule.valueMin && value <= rule.valueMax) {
            ++m_matchCount;
            ++m_totalMatches;
            ++m_totalActionsExecuted;
            ++m_totalTriggersFired;
            if (i < m_ruleMatchCounts.size()) {
                ++m_ruleMatchCounts[i];
            }
            m_lastMatchTimer.start();
            m_hasMatched = true;
            emit triggered(i, rule.name);
            emit actionRequired(static_cast<int>(rule.actionType), rule.actionData);
        }
    }
}

/** @brief 添加一条触发器规则并更新峰值活跃计数 @param rule 规则配置 */
void TriggerEngine::addRule(const TriggerRuleConfig& rule)
{
    m_rules.append(rule);
    m_ruleMatchCounts.append(0);
    /* 更新同时启用规则数峰值 */
    if (rule.enabled) {
        int activeCount = 0;
        for (const auto& r : m_rules) {
            if (r.enabled) ++activeCount;
        }
        if (activeCount > m_peakRulesActive) {
            m_peakRulesActive = activeCount;
        }
    }
}

/** @brief 移除指定索引的规则 @param index 规则索引 */
void TriggerEngine::removeRule(int index)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules.removeAt(index);
        if (index < m_ruleMatchCounts.size()) {
            m_ruleMatchCounts.removeAt(index);
        }
    }
}

/** @brief 设置所有规则的启用/禁用状态 @param enabled true=启用，false=禁用 */
void TriggerEngine::setRulesEnabled(bool enabled)
{
    m_enabled = enabled;
}

/** @brief 设置指定规则的启用/禁用状态，启用时递增totalRulesActive并更新峰值 @param index 规则索引 @param enabled true=启用，false=禁用 */
void TriggerEngine::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_rules.size()) {
        /* 仅当从禁用切换到启用时计数 */
        if (enabled && !m_rules[index].enabled) {
            ++m_totalRulesActive;
        }
        m_rules[index].enabled = enabled;
        /* 更新同时启用规则数峰值 */
        int activeCount = 0;
        for (const auto& r : m_rules) {
            if (r.enabled) ++activeCount;
        }
        if (activeCount > m_peakRulesActive) {
            m_peakRulesActive = activeCount;
        }
    }
}

/** @brief 获取所有规则列表 @return 规则配置列表的常引用 */
const QList<TriggerRuleConfig>& TriggerEngine::rules() const
{
    return m_rules;
}

/** @brief 清空所有规则和匹配计数，用于重新加载配置前清空旧数据 */
void TriggerEngine::clearRules()
{
    m_rules.clear();
    m_ruleMatchCounts.clear();
}

// 统计查询和重置方法见 TriggerEngineStats.cpp

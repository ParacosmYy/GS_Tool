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

/** @brief 添加一条触发器规则 @param rule 规则配置 */
void TriggerEngine::addRule(const TriggerRuleConfig& rule)
{
    m_rules.append(rule);
    m_ruleMatchCounts.append(0);
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

/** @brief 设置指定规则的启用/禁用状态 @param index 规则索引 @param enabled true=启用，false=禁用 */
void TriggerEngine::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules[index].enabled = enabled;
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

/** @brief 获取累计成功匹配次数 @return 匹配次数 */
int TriggerEngine::matchCount() const
{
    return m_matchCount;
}

/** @brief 获取上次匹配距现在的毫秒数 @return 距上次匹配的毫秒数，无匹配返回-1 */
qint64 TriggerEngine::msSinceLastMatch() const
{
    if (!m_hasMatched) {
        return -1;
    }
    return m_lastMatchTimer.elapsed();
}

/** @brief 重置统计计数(不重置规则) */
void TriggerEngine::resetStatistics()
{
    m_matchCount = 0;
    m_hasMatched = false;
    m_ruleMatchCounts.fill(0);
}

/** @brief 获取指定规则的匹配次数 @param index 规则索引 @return 该规则命中次数，无效索引返回0 */
int TriggerEngine::ruleMatchCount(int index) const
{
    if (index >= 0 && index < m_ruleMatchCounts.size()) {
        return m_ruleMatchCounts.at(index);
    }
    return 0;
}

// ---- 扩展统计 getter ----

/** @brief 获取总评估次数 @return 总评估次数 */
quint64 TriggerEngine::totalEvaluations() const
{
    return m_totalEvaluations;
}

/** @brief 获取总匹配成功次数（quint64精度） @return 总匹配次数 */
quint64 TriggerEngine::totalMatches() const
{
    return m_totalMatches;
}

/** @brief 获取总动作执行次数 @return 总动作执行次数 */
quint64 TriggerEngine::totalActionsExecuted() const
{
    return m_totalActionsExecuted;
}

/** @brief 获取总错误次数 @return 总错误次数 */
quint64 TriggerEngine::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 获取总规则评估次数 @return 规则评估总次数 */
quint64 TriggerEngine::totalTriggersEvaluated() const
{
    return m_totalTriggersEvaluated;
}

/** @brief 获取总触发器命中次数 @return 触发器命中总次数 */
quint64 TriggerEngine::totalTriggersFired() const
{
    return m_totalTriggersFired;
}

/** @brief 获取总跳过禁用规则的次数 @return 跳过禁用规则总次数 */
quint64 TriggerEngine::totalTriggersDisabled() const
{
    return m_totalTriggersDisabled;
}

/** @brief 获取总动作执行错误次数 @return 动作执行错误总次数 */
quint64 TriggerEngine::totalActionErrors() const
{
    return m_totalActionErrors;
}

/** @brief 重置所有扩展统计计数器为初始值(不影响规则列表和启用状态) */
void TriggerEngine::resetStats()
{
    m_totalEvaluations = 0;
    m_totalMatches = 0;
    m_totalActionsExecuted = 0;
    m_totalErrors = 0;
    m_totalTriggersEvaluated = 0;
    m_totalTriggersFired = 0;
    m_totalTriggersDisabled = 0;
    m_totalActionErrors = 0;
    resetStatistics();
}

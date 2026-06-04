/**
 * @file TerminalHighlighter.cpp
 * @brief 终端多模式高亮器实现 - 多正则规则并行高亮
 *
 * highlightBlock 中按顺序应用所有启用的规则，后匹配的规则覆盖先匹配的格式。
 * 无启用的规则时回退到单规则兼容模式的默认正则。
 */

#include "terminal/filter/TerminalHighlighter.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造函数，初始化默认高亮格式 @param parent 关联的文本文档 */
TerminalHighlighter::TerminalHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_color(ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight))
{
    m_format = buildFormat(m_color);
}

// ---- 单规则兼容接口 ----

/** @brief 设置单条高亮正则模式并重新高亮 @param pattern 正则表达式字符串 */
void TerminalHighlighter::setPattern(const QString &pattern)
{
    m_pattern = pattern;
    m_regex.setPattern(pattern);
    ++m_totalRuleChanges;
    rehighlight();
}

/** @brief 设置默认高亮颜色并重新高亮 @param color 高亮前景色 */
void TerminalHighlighter::setHighlightColor(const QColor &color)
{
    m_color = color;
    m_format = buildFormat(color);
    ++m_totalRuleChanges;
    ++m_totalColorChanges; ///< 统计: 颜色变更
    rehighlight();
}

// ---- 多规则接口 ----

/**
 * @brief 添加一条高亮规则
 * @param pattern 正则表达式字符串
 * @param color 高亮颜色
 * @param enabled 是否默认启用
 * @return 规则索引(从0开始)，-1表示正则无效
 */
int TerminalHighlighter::addRule(const QString& pattern, const QColor& color, bool enabled)
{
    QRegularExpression re(pattern);
    if (!re.isValid()) {
        return -1;
    }

    HighlightRule rule;
    rule.pattern = pattern;
    rule.regex = re;
    rule.color = color;
    rule.format = buildFormat(color);
    rule.enabled = enabled;

    m_rules.append(rule);
    int index = m_rules.size() - 1;
    ++m_totalRuleChanges;
    rehighlight();
    return index;
}

/**
 * @brief 移除指定索引的高亮规则
 * @param index 规则索引
 * @return true 成功移除，false 索引无效
 */
bool TerminalHighlighter::removeRule(int index)
{
    if (index < 0 || index >= m_rules.size()) {
        return false;
    }
    m_rules.removeAt(index);
    ++m_totalRuleChanges;
    rehighlight();
    return true;
}

/**
 * @brief 更新指定索引的高亮规则正则表达式
 * @param index 规则索引
 * @param pattern 新的正则表达式
 * @return true 更新成功
 */
bool TerminalHighlighter::updateRule(int index, const QString& pattern)
{
    if (index < 0 || index >= m_rules.size()) {
        return false;
    }

    QRegularExpression re(pattern);
    if (!re.isValid()) {
        return false;
    }

    m_rules[index].pattern = pattern;
    m_rules[index].regex = re;
    ++m_totalRuleChanges;
    rehighlight();
    return true;
}

/**
 * @brief 设置指定规则的高亮颜色
 * @param index 规则索引
 * @param color 高亮颜色
 */
void TerminalHighlighter::setRuleColor(int index, const QColor& color)
{
    if (index < 0 || index >= m_rules.size()) return;
    m_rules[index].color = color;
    m_rules[index].format = buildFormat(color);
    ++m_totalRuleChanges;
    ++m_totalColorChanges; ///< 统计: 颜色变更
    rehighlight();
}

/**
 * @brief 设置指定规则的启用状态
 * @param index 规则索引
 * @param enabled 是否启用
 */
void TerminalHighlighter::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules[index].enabled = enabled;
        ++m_totalRuleChanges;
        rehighlight();
    }
}

/** @brief 获取所有高亮规则(只读) @return HighlightRule向量的const引用 */
const QVector<HighlightRule>& TerminalHighlighter::rules() const
{
    return m_rules;
}

/** @brief 清除所有高亮规则 */
void TerminalHighlighter::clearRules()
{
    m_rules.clear();
    ++m_totalRuleChanges;
    rehighlight();
}

// ---- 高亮渲染/统计接口见 TerminalHighlighterHighlight.cpp ----
// highlightBlock / buildFormat / totalRulesActive / resetStatistics

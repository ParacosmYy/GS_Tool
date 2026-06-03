/**
 * @file TerminalFilter.cpp
 * @brief 终端多模式过滤器实现 - 多正则并行过滤 + 包含/排除模式 + 时间戳范围
 *
 * 过滤逻辑:
 *   - 包含模式(Include): 任一启用规则匹配则通过，全部不匹配则阻塞
 *   - 排除模式(Exclude): 任一启用规则匹配则阻塞，全部不匹配则通过
 *   - 无启用规则时全部通过(不过滤)
 *   - 时间戳范围独立判断，与正则规则取交集
 */

#include "terminal/filter/TerminalFilter.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造函数，初始化默认正则(大小写不敏感) @param parent 父对象 */
TerminalFilter::TerminalFilter(QObject *parent)
    : QObject(parent)
    , m_caseSensitive(false)
{
}

// ---- 单模式兼容接口 ----

/** @brief 设置单条正则表达式模式并编译 @param pattern 正则表达式字符串 @return true=模式有效，false=编译失败 */
bool TerminalFilter::setPattern(const QString &pattern)
{
    m_pattern = pattern;
    m_regex = compileRegex(pattern, m_caseSensitive);

    if (!m_regex.isValid()) {
        emit patternError(m_regex.errorString());
        return false;
    }
    return true;
}

/** @brief 对文本执行正则匹配(单模式兼容) @param text 待匹配文本 @return true=匹配成功 */
bool TerminalFilter::match(const QString &text) const
{
    if (!m_regex.isValid()) {
        return false;
    }
    bool result = m_regex.match(text).hasMatch();
    if (result) {
        ++m_matchCount;
        m_lastMatch = text;
    }
    return result;
}

/** @brief 提取捕获组内容(单模式兼容) @param text 待提取文本 @return 捕获组字符串列表 */
QStringList TerminalFilter::captureGroups(const QString &text) const
{
    if (!m_regex.isValid()) {
        return {};
    }

    QStringList result;
    QRegularExpressionMatch matchObj = m_regex.match(text);
    if (matchObj.hasMatch()) {
        for (int i = 0; i <= matchObj.lastCapturedIndex(); ++i) {
            result.append(matchObj.captured(i));
        }
    }
    return result;
}

/** @brief 设置大小写敏感并重新编译单模式正则 @param sensitive true=区分大小写 */
void TerminalFilter::setCaseSensitive(bool sensitive)
{
    m_caseSensitive = sensitive;
    // 重新编译正则以应用新选项
    if (!m_pattern.isEmpty()) {
        setPattern(m_pattern);
    }
}

// ---- 多模式过滤接口 ----

/**
 * @brief 添加一条过滤规则
 * @param pattern 正则表达式字符串
 * @param caseSensitive 是否区分大小写
 * @param enabled 是否默认启用
 * @return 规则索引(从0开始)，-1表示正则无效
 */
int TerminalFilter::addRule(const QString& pattern, bool caseSensitive, bool enabled)
{
    QRegularExpression re = compileRegex(pattern, caseSensitive);
    if (!re.isValid()) {
        emit patternError(re.errorString());
        return -1;
    }

    FilterRule rule;
    rule.pattern = pattern;
    rule.regex = re;
    rule.enabled = enabled;
    rule.caseSensitive = caseSensitive;

    m_rules.append(rule);
    int index = m_rules.size() - 1;
    emit filterRulesChanged();
    return index;
}

/**
 * @brief 移除指定索引的过滤规则
 * @param index 规则索引
 * @return true 成功移除，false 索引无效
 */
bool TerminalFilter::removeRule(int index)
{
    if (index < 0 || index >= m_rules.size()) {
        return false;
    }
    m_rules.removeAt(index);
    emit filterRulesChanged();
    return true;
}

/**
 * @brief 更新指定索引的过滤规则
 * @param index 规则索引
 * @param pattern 新的正则表达式
 * @return true 更新成功，false 索引无效或正则无效
 */
bool TerminalFilter::updateRule(int index, const QString& pattern)
{
    if (index < 0 || index >= m_rules.size()) {
        return false;
    }

    QRegularExpression re = compileRegex(pattern, m_rules[index].caseSensitive);
    if (!re.isValid()) {
        emit patternError(re.errorString());
        return false;
    }

    m_rules[index].pattern = pattern;
    m_rules[index].regex = re;
    emit filterRulesChanged();
    return true;
}

/**
 * @brief 设置指定规则的启用状态
 * @param index 规则索引
 * @param enabled 是否启用
 */
void TerminalFilter::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_rules.size()) {
        m_rules[index].enabled = enabled;
        emit filterRulesChanged();
    }
}

/**
 * @brief 设置指定规则的大小写敏感
 * @param index 规则索引
 * @param sensitive 是否区分大小写
 */
void TerminalFilter::setRuleCaseSensitive(int index, bool sensitive)
{
    if (index < 0 || index >= m_rules.size()) return;
    m_rules[index].caseSensitive = sensitive;
    // 重新编译该规则的正则
    m_rules[index].regex = compileRegex(m_rules[index].pattern, sensitive);
    emit filterRulesChanged();
}

/** @brief 获取所有过滤规则(只读) @return FilterRule向量的const引用 */
const QVector<FilterRule>& TerminalFilter::rules() const
{
    return m_rules;
}

/** @brief 获取当前过滤模式 @return FilterMode枚举值 */
FilterMode TerminalFilter::filterMode() const
{
    return m_filterMode;
}

/**
 * @brief 设置过滤模式(包含/排除)
 * @param mode 过滤模式
 */
void TerminalFilter::setFilterMode(FilterMode mode)
{
    if (m_filterMode != mode) {
        m_filterMode = mode;
        emit filterRulesChanged();
    }
}

/** @brief 清除所有过滤规则 */
void TerminalFilter::clearRules()
{
    m_rules.clear();
    emit filterRulesChanged();
}

// ---- 多模式过滤判断 ----

/**
 * @brief 多模式过滤判断: 文本是否通过当前规则集
 *
 * Include模式: 任一启用的规则匹配则通过，全部不匹配则阻塞
 * Exclude模式: 任一启用的规则匹配则阻塞，全部不匹配则通过
 * 无启用的规则时始终通过(不过滤)
 *
 * @param text 待检查文本
 * @return true 文本通过过滤应显示，false 文本被过滤应隐藏
 */
bool TerminalFilter::filter(const QString& text)
{
    // 收集所有启用的规则中匹配的结果
    bool hasEnabled = false;
    bool anyMatched = false;

    for (const auto& rule : m_rules) {
        if (!rule.enabled) continue;
        hasEnabled = true;
        if (rule.regex.isValid() && rule.regex.match(text).hasMatch()) {
            anyMatched = true;
            break;  // 短路: 已知有匹配，无需继续
        }
    }

    // 无启用的规则时不过滤，全部通过
    if (!hasEnabled) {
        ++m_filterPassCount;
        return true;
    }

    bool passed;
    if (m_filterMode == FilterMode::Include) {
        // 包含模式: 匹配则通过
        passed = anyMatched;
    } else {
        // 排除模式: 匹配则阻塞
        passed = !anyMatched;
    }

    if (passed) {
        ++m_filterPassCount;
    } else {
        ++m_filterBlockCount;
    }
    return passed;
}

/**
 * @brief 多模式过滤判断: 文本+时间戳是否通过当前规则集
 *
 * 同时检查正则规则和时间戳范围，两者都通过才返回true。
 *
 * @param text 待检查文本
 * @param timestamp 数据时间戳(epoch毫秒)
 * @return true 文本通过过滤应显示，false 文本被过滤应隐藏
 */
bool TerminalFilter::filter(const QString& text, qint64 timestamp)
{
    // 先检查时间戳范围
    if (!checkTimestampRange(timestamp)) {
        ++m_filterBlockCount;
        return false;
    }

    // 再检查正则规则
    return filter(text);
}

// ---- 时间戳范围过滤 ----

/**
 * @brief 设置时间戳过滤范围
 * @param from 起始时间(含)，无效QDateTime表示不限制
 * @param to 结束时间(含)，无效QDateTime表示不限制
 */
void TerminalFilter::setTimestampRange(const QDateTime& from, const QDateTime& to)
{
    if (from.isValid()) {
        m_timestampFrom = from;
        m_timestampFromMs = from.toMSecsSinceEpoch();
    } else {
        m_timestampFrom = QDateTime();
        m_timestampFromMs = 0;
    }

    if (to.isValid()) {
        m_timestampTo = to;
        m_timestampToMs = to.toMSecsSinceEpoch();
    } else {
        m_timestampTo = QDateTime();
        m_timestampToMs = 0;
    }

    emit filterRulesChanged();
}

/** @brief 清除时间戳过滤范围 */
void TerminalFilter::clearTimestampRange()
{
    m_timestampFrom = QDateTime();
    m_timestampTo = QDateTime();
    m_timestampFromMs = 0;
    m_timestampToMs = 0;
    emit filterRulesChanged();
}

/** @brief 是否启用了时间戳过滤 @return true 起始或结束时间已设置 */
bool TerminalFilter::hasTimestampFilter() const
{
    return m_timestampFrom.isValid() || m_timestampTo.isValid();
}

/** @brief 获取时间戳起始时间 @return 起始QDateTime(无效表示不限制) */
QDateTime TerminalFilter::timestampFrom() const
{
    return m_timestampFrom;
}

/** @brief 获取时间戳结束时间 @return 结束QDateTime(无效表示不限制) */
QDateTime TerminalFilter::timestampTo() const
{
    return m_timestampTo;
}

// ---- 高亮颜色(兼容接口) ----

/** @brief 返回高亮颜色名称 @return 颜色HEX字符串 */
QString TerminalFilter::highlightColor() const
{
    return ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight).name();
}

// ---- 统计接口 ----

/** @brief 获取累计匹配次数(单模式兼容) @return 匹配总次数 */
quint64 TerminalFilter::matchCount() const
{
    return m_matchCount;
}

/** @brief 获取最近一次匹配的文本 @return 最后匹配的文本 */
QString TerminalFilter::lastMatchText() const
{
    return m_lastMatch;
}

/** @brief 获取过滤通过的总行数 @return 通过计数 */
quint64 TerminalFilter::filterPassCount() const
{
    return m_filterPassCount;
}

/** @brief 获取过滤阻塞的总行数 @return 阻塞计数 */
quint64 TerminalFilter::filterBlockCount() const
{
    return m_filterBlockCount;
}

/** @brief 重置所有统计计数器为零 */
void TerminalFilter::resetStatistics()
{
    m_matchCount = 0;
    m_lastMatch.clear();
    m_filterPassCount = 0;
    m_filterBlockCount = 0;
}

// ---- 私有方法 ----

/**
 * @brief 编译正则表达式并设置选项
 * @param pattern 正则字符串
 * @param caseSensitive 是否区分大小写
 * @return 编译后的QRegularExpression对象(可能无效)
 */
QRegularExpression TerminalFilter::compileRegex(const QString& pattern, bool caseSensitive) const
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    QRegularExpression re(pattern, options);
    return re;
}

/**
 * @brief 检查时间戳是否在指定范围内
 * @param timestamp epoch毫秒时间戳
 * @return true 在范围内或未设置范围
 */
bool TerminalFilter::checkTimestampRange(qint64 timestamp) const
{
    if (m_timestampFromMs > 0 && timestamp < m_timestampFromMs) {
        return false;
    }
    if (m_timestampToMs > 0 && timestamp > m_timestampToMs) {
        return false;
    }
    return true;
}

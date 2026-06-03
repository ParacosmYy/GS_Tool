/**
 * @file TerminalFilter.h
 * @brief 终端多模式过滤器 - 支持正则/包含/排除模式和时间戳范围过滤
 *
 * 多正则模式并行过滤(AND/OR逻辑)，包含/排除模式切换，时间戳范围过滤。
 * 协作: TerminalWidget(持有), TerminalHighlighter(独立高亮)
 */

#ifndef TERMINALFILTER_H
#define TERMINALFILTER_H

#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>

/** @brief 单条过滤规则 */
struct FilterRule {
    QString pattern;                ///< 正则表达式字符串
    QRegularExpression regex;       ///< 编译后的正则对象
    bool enabled = true;            ///< 是否启用此规则
    bool caseSensitive = false;     ///< 是否区分大小写
};

/** @brief 过滤器工作模式 */
enum class FilterMode {
    Include,    ///< 包含模式: 仅显示匹配任一规则的行
    Exclude     ///< 排除模式: 隐藏匹配任一规则的行
};

/**
 * @class TerminalFilter
 * @brief 终端多模式过滤器 - 多正则并行过滤 + 包含/排除模式 + 时间戳范围
 *
 * Include模式: 任一启用规则匹配则通过; Exclude模式: 任一匹配则阻塞。
 * 时间戳范围独立于正则规则，两者取交集。
 * 统计: filterPassCount(通过行数), filterBlockCount(阻塞行数)
 */
class TerminalFilter : public QObject
{
    Q_OBJECT

public:
    explicit TerminalFilter(QObject *parent = nullptr);

    // ---- 单模式兼容接口 ----
    bool setPattern(const QString &pattern);        ///< 设置单条正则并编译 @return true=有效
    bool match(const QString &text) const;          ///< 单模式正则匹配 @return true=匹配
    QStringList captureGroups(const QString &text) const; ///< 提取捕获组
    void setCaseSensitive(bool sensitive);          ///< 设置大小写敏感

    // ---- 多模式过滤接口 ----
    int addRule(const QString& pattern, bool caseSensitive = false, bool enabled = true); ///< 添加规则 @return 索引/-1
    bool removeRule(int index);                     ///< 移除规则 @return true=成功
    bool updateRule(int index, const QString& pattern); ///< 更新规则正则 @return true=成功
    void setRuleEnabled(int index, bool enabled);   ///< 设置规则启用状态
    void setRuleCaseSensitive(int index, bool sensitive); ///< 设置规则大小写敏感
    const QVector<FilterRule>& rules() const;       ///< 获取所有规则(只读)
    FilterMode filterMode() const;                  ///< 获取过滤模式
    void setFilterMode(FilterMode mode);            ///< 设置过滤模式(包含/排除)
    void clearRules();                              ///< 清除所有规则

    // ---- 多模式过滤判断 ----
    bool filter(const QString& text);               ///< 文本是否通过规则集 @return true=显示
    bool filter(const QString& text, qint64 timestamp); ///< 文本+时间戳是否通过 @return true=显示

    // ---- 时间戳范围过滤 ----
    void setTimestampRange(const QDateTime& from, const QDateTime& to); ///< 设置时间范围(无效=不限)
    void clearTimestampRange();                     ///< 清除时间范围
    bool hasTimestampFilter() const;                ///< 是否启用了时间戳过滤
    QDateTime timestampFrom() const;                ///< 起始时间(无效=不限制)
    QDateTime timestampTo() const;                  ///< 结束时间(无效=不限制)

    QString highlightColor() const;                 ///< 高亮颜色名称(兼容接口)

    // ---- 统计接口 ----
    quint64 matchCount() const;                     ///< 匹配次数(单模式兼容)
    QString lastMatchText() const;                  ///< 最近匹配文本
    quint64 filterPassCount() const;                ///< 过滤通过总行数
    quint64 filterBlockCount() const;               ///< 过滤阻塞总行数
    void resetStatistics();                         ///< 重置所有统计

signals:
    void patternError(const QString &errorMessage); ///< 正则错误信号
    void filterRulesChanged();                      ///< 规则变化信号(添加/移除/更新/模式切换)

private:
    QRegularExpression compileRegex(const QString& pattern, bool caseSensitive) const; ///< 编译正则
    bool checkTimestampRange(qint64 timestamp) const; ///< 检查时间戳范围

    QString m_pattern;                              ///< 单模式正则(兼容接口)
    bool m_caseSensitive = false;                   ///< 单模式大小写(兼容接口)
    QRegularExpression m_regex;                     ///< 单模式正则对象(兼容接口)

    QVector<FilterRule> m_rules;                    ///< 多模式过滤规则列表
    FilterMode m_filterMode = FilterMode::Include;  ///< 过滤模式
    QDateTime m_timestampFrom;                      ///< 起始时间(无效=不限)
    QDateTime m_timestampTo;                        ///< 结束时间(无效=不限)
    qint64 m_timestampFromMs = 0;                   ///< 起始epoch毫秒(0=不限)
    qint64 m_timestampToMs = 0;                     ///< 结束epoch毫秒(0=不限)

    mutable quint64 m_matchCount = 0;               ///< 单模式匹配计数
    mutable QString m_lastMatch;                    ///< 最近匹配文本
    quint64 m_filterPassCount = 0;                  ///< 过滤通过行数
    quint64 m_filterBlockCount = 0;                 ///< 过滤阻塞行数
};

#endif // TERMINALFILTER_H

/**
 * @file TerminalHighlighter.h
 * @brief 终端多模式高亮器 - 支持多正则规则和高亮颜色
 *
 * 基于 QSyntaxHighlighter 的终端文本正则高亮组件。
 * 支持多条独立的正则高亮规则，每条规则可设置不同的高亮颜色。
 * 统计计数器跟踪高亮匹配次数和过滤通过/阻塞行数。
 */

#ifndef TERMINALHIGHLIGHTER_H
#define TERMINALHIGHLIGHTER_H

#include <QColor>
#include <QtGlobal>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QVector>
#include <QPair>

/** @brief 单条高亮规则 */
struct HighlightRule {
    QString pattern;                ///< 正则表达式字符串
    QRegularExpression regex;       ///< 编译后的正则对象
    QTextCharFormat format;         ///< 高亮文本格式
    QColor color;                   ///< 高亮颜色
    bool enabled = true;            ///< 是否启用
};

/**
 * @class TerminalHighlighter
 * @brief 终端多模式高亮器 - 多正则规则并行高亮
 *
 * 支持多条独立的高亮规则，每条规则有独立的正则表达式和高亮颜色。
 * 在 highlightBlock 中按顺序应用所有启用的规则，后匹配的规则覆盖先匹配的。
 */
class TerminalHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit TerminalHighlighter(QTextDocument *parent = nullptr); ///< 构造函数

    // ---- 单规则兼容接口 ----

    void setPattern(const QString &pattern);   ///< 设置单条高亮正则模式
    void setHighlightColor(const QColor &color); ///< 设置默认高亮颜色

    // ---- 多规则接口 ----

    int addRule(const QString& pattern, const QColor& color, bool enabled = true); ///< 添加高亮规则，返回规则索引(-1=正则无效)
    bool removeRule(int index);      ///< 移除指定索引的高亮规则
    bool updateRule(int index, const QString& pattern); ///< 更新指定索引的高亮规则
    void setRuleColor(int index, const QColor& color);  ///< 设置指定规则的高亮颜色
    void setRuleEnabled(int index, bool enabled);       ///< 设置指定规则的启用状态
    const QVector<HighlightRule>& rules() const; ///< 获取所有高亮规则(只读)
    void clearRules();               ///< 清除所有高亮规则

    // ---- 统计接口 ----

    quint64 totalHighlights() const { return m_totalHighlights; }  ///< 高亮匹配总次数
    quint64 totalRuleChanges() const { return m_totalRuleChanges; } ///< 规则变更总次数
    quint64 filterPassCount() const { return m_filterPassCount; }  ///< 过滤通过的总行数(匹配到高亮的行)
    quint64 filterBlockCount() const { return m_filterBlockCount; } ///< 过滤阻塞的总行数(未匹配任何规则的行)
    quint64 totalRegexMatches() const { return m_totalRegexMatches; } ///< 正则匹配总次数
    quint64 totalColorChanges() const { return m_totalColorChanges; } ///< 颜色变更总次数
    quint64 totalRulesActive() const; ///< 当前活跃规则数
    quint64 totalHighlightErrors() const { return m_highlightErrors; } ///< 高亮错误总次数
    void resetStatistics();           ///< 重置所有统计计数器

protected:
    void highlightBlock(const QString &text) override; ///< 高亮文本块中所有匹配内容

private:
    QTextCharFormat buildFormat(const QColor& color) const; ///< 构建高亮文本格式

    // ---- 单规则兼容成员 ----
    QString m_pattern;          ///< 单规则正则字符串(兼容接口)
    QColor m_color;             ///< 默认高亮颜色
    QRegularExpression m_regex; ///< 单规则编译后正则(兼容接口)
    QTextCharFormat m_format;   ///< 默认高亮文本格式

    // ---- 多规则成员 ----
    QVector<HighlightRule> m_rules; ///< 多规则高亮列表

    // ---- 统计计数 ----
    quint64 m_totalHighlights = 0;  ///< 高亮匹配总次数
    quint64 m_totalRuleChanges = 0; ///< 规则变更总次数
    quint64 m_filterPassCount = 0;  ///< 过滤通过行数
    quint64 m_filterBlockCount = 0; ///< 过滤阻塞行数
    quint64 m_totalRegexMatches = 0; ///< 正则匹配总次数
    quint64 m_totalColorChanges = 0; ///< 颜色变更总次数
    quint64 m_highlightErrors = 0;   ///< 高亮错误总次数
};

#endif // TERMINALHIGHLIGHTER_H

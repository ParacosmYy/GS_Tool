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

/**
 * @brief 单条高亮规则
 */
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
 *
 * 统计计数器:
 *   - totalHighlights: 高亮匹配总次数
 *   - totalRuleChanges: 规则变更总次数(添加/移除/更新)
 *   - filterPassCount: 通过过滤的行数(匹配到高亮的行)
 *   - filterBlockCount: 被过滤阻塞的行数(未匹配任何规则的行)
 */
class TerminalHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 关联的文本文档
     */
    explicit TerminalHighlighter(QTextDocument *parent = nullptr);

    // ---- 单规则兼容接口 ----

    /**
     * @brief 设置单条高亮正则模式
     * @param pattern 正则表达式字符串
     */
    void setPattern(const QString &pattern);

    /**
     * @brief 设置默认高亮颜色
     * @param color 高亮前景色
     */
    void setHighlightColor(const QColor &color);

    // ---- 多规则接口 ----

    /**
     * @brief 添加一条高亮规则
     * @param pattern 正则表达式字符串
     * @param color 高亮颜色
     * @param enabled 是否默认启用
     * @return 规则索引(从0开始)，-1表示正则无效
     */
    int addRule(const QString& pattern, const QColor& color, bool enabled = true);

    /**
     * @brief 移除指定索引的高亮规则
     * @param index 规则索引
     * @return true 成功移除，false 索引无效
     */
    bool removeRule(int index);

    /**
     * @brief 更新指定索引的高亮规则
     * @param index 规则索引
     * @param pattern 新的正则表达式
     * @return true 更新成功
     */
    bool updateRule(int index, const QString& pattern);

    /**
     * @brief 设置指定规则的高亮颜色
     * @param index 规则索引
     * @param color 高亮颜色
     */
    void setRuleColor(int index, const QColor& color);

    /**
     * @brief 设置指定规则的启用状态
     * @param index 规则索引
     * @param enabled 是否启用
     */
    void setRuleEnabled(int index, bool enabled);

    /**
     * @brief 获取所有高亮规则(只读)
     * @return HighlightRule向量的const引用
     */
    const QVector<HighlightRule>& rules() const;

    /** @brief 清除所有高亮规则 */
    void clearRules();

    // ---- 统计接口 ----

    /** @brief 获取高亮匹配总次数 */
    quint64 totalHighlights() const { return m_totalHighlights; }

    /** @brief 获取规则变更总次数 */
    quint64 totalRuleChanges() const { return m_totalRuleChanges; }

    /** @brief 获取过滤通过的总行数(匹配到高亮的行) */
    quint64 filterPassCount() const { return m_filterPassCount; }

    /** @brief 获取过滤阻塞的总行数(未匹配任何规则的行) */
    quint64 filterBlockCount() const { return m_filterBlockCount; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /**
     * @brief 高亮文本块中所有匹配内容(应用所有启用的规则)
     * @param text 待处理文本
     */
    void highlightBlock(const QString &text) override;

private:
    /**
     * @brief 构建高亮文本格式
     * @param color 高亮颜色
     * @return 配置好的QTextCharFormat
     */
    QTextCharFormat buildFormat(const QColor& color) const;

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
    quint64 m_filterPassCount = 0;  ///< 过滤通过行数(匹配到高亮)
    quint64 m_filterBlockCount = 0; ///< 过滤阻塞行数(未匹配高亮)
};

#endif // TERMINALHIGHLIGHTER_H

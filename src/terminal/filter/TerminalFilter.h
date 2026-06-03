/**
 * @file TerminalFilter.h
 * @brief 终端正则过滤器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 基于正则表达式的终端数据过滤，支持大小写敏感控制和捕获组提取。
 */

#ifndef TERMINALFILTER_H
#define TERMINALFILTER_H

#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

/**
 * @class TerminalFilter
 * @brief 终端正则过滤器，提供正则匹配与捕获组提取功能
 */
class TerminalFilter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TerminalFilter(QObject *parent = nullptr);

    /**
     * @brief 设置正则表达式模式
     * @param pattern 正则表达式字符串
     * @return 设置是否成功（true=有效模式）
     */
    bool setPattern(const QString &pattern);

    /**
     * @brief 对文本执行正则匹配
     * @param text 待匹配文本
     * @return 是否匹配成功
     */
    bool match(const QString &text) const;

    /**
     * @brief 提取捕获组内容
     * @param text 待提取文本
     * @return 捕获组字符串列表
     */
    QStringList captureGroups(const QString &text) const;

    /**
     * @brief 设置大小写敏感
     * @param sensitive 是否区分大小写
     */
    void setCaseSensitive(bool sensitive);

    /**
     * @brief 获取高亮颜色
     * @return 高亮颜色名称
     */
    QString highlightColor() const;

    /** @brief 获取匹配次数 */
    quint64 matchCount() const;

    /** @brief 获取最近一次匹配的文本 */
    QString lastMatchText() const;

    /** @brief 重置匹配统计 */
    void resetStatistics();

signals:
    /**
     * @brief 正则表达式错误信号
     * @param errorMessage 错误描述
     */
    void patternError(const QString &errorMessage);

private:
    QString m_pattern;              ///< 正则表达式字符串
    bool m_caseSensitive = false;   ///< 大小写敏感标志
    QRegularExpression m_regex;     ///< 编译后的正则对象

    /** @brief 累计匹配次数 */
    mutable quint64 m_matchCount = 0;
    /** @brief 最近一次匹配的文本 */
    mutable QString m_lastMatch;
};

#endif // TERMINALFILTER_H

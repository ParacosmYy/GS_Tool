/**
 * @file TerminalHighlighter.h
 * @brief 终端正则高亮器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 基于 QSyntaxHighlighter 的终端文本正则高亮组件。
 */

#ifndef TERMINALHIGHLIGHTER_H
#define TERMINALHIGHLIGHTER_H

#include <QColor>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>

/**
 * @class TerminalHighlighter
 * @brief 终端文本正则高亮器，匹配文本自动着色
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

    /**
     * @brief 设置高亮正则模式
     * @param pattern 正则表达式字符串
     */
    void setPattern(const QString &pattern);

    /**
     * @brief 设置高亮颜色
     * @param color 高亮前景色
     */
    void setHighlightColor(const QColor &color);

protected:
    /**
     * @brief 高亮匹配文本块
     * @param text 待处理文本
     */
    void highlightBlock(const QString &text) override;

private:
    QString m_pattern;          ///< 正则表达式字符串
    QColor m_color;             ///< 高亮颜色
    QRegularExpression m_regex; ///< 编译后正则对象
    QTextCharFormat m_format;   ///< 高亮文本格式
};

#endif // TERMINALHIGHLIGHTER_H

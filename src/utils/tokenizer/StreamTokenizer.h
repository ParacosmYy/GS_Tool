/**
 * @file StreamTokenizer.h
 * @brief 流式分词器(Stream Tokenizer)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QSet>
#include <QMap>

/**
 * @class StreamTokenizer
 * @brief 流式分词器 — 支持自定义分隔符和规则的文本分词
 *
 * 支持多种分词模式: 按字符、按正则、按空白符。
 * 可配置停止词过滤和小写转换。
 */
class StreamTokenizer : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTokenized = 0;    /**< 总分词次数 */
        int totalTokens = 0;       /**< 总token数 */
        int totalFiltered = 0;     /**< 过滤的token数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit StreamTokenizer(QObject* parent = nullptr);

    /**
     * @brief 按分隔符分词
     * @param text 输入文本
     * @param delimiters 分隔符集合(默认空白符)
     * @return token列表
     */
    QVector<QString> tokenize(const QString& text,
                               const QString& delimiters = QStringLiteral(" \t\n\r")) const;

    /**
     * @brief N-gram分词
     * @param text 输入文本
     * @param n n-gram长度
     * @return n-gram列表
     */
    QVector<QString> ngrams(const QString& text, int n) const;

    /** @brief 设置停止词列表 */
    void setStopWords(const QSet<QString>& words);

    /** @brief 过滤停止词 */
    QVector<QString> filterStopWords(const QVector<QString>& tokens) const;

    /** @brief 转小写 */
    static QVector<QString> toLower(const QVector<QString>& tokens);

    /** @brief 去重 */
    static QVector<QString> unique(const QVector<QString>& tokens);

    /** @brief 词频统计 */
    static QMap<QString, int> frequency(const QVector<QString>& tokens);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分词完成信号 */
    void tokenizationCompleted(int tokenCount);

private:
    QSet<QString> m_stopWords;
    mutable Stats m_stats;
    mutable double m_timeSum;
};

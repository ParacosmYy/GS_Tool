/**
 * @file StreamTokenizer.h
 * @brief 流式分词器 — 可配置分隔符的增量分词
 *
 * 功能: 支持自定义分隔符/引用符/转义符的流式分词器，
 *       适用于协议解析/CSV处理，统计分词数/行数/耗时。
 */
#ifndef STREAMTOKENIZER_H
#define STREAMTOKENIZER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QStringList>

class StreamTokenizer : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalTokensExtracted = 0;
        quint64 totalBytesProcessed = 0;
        quint64 totalLinesProcessed = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit StreamTokenizer(QObject* parent = nullptr);

    /** @brief 设置分隔符 @param delimiters 分隔符集合 */
    void setDelimiters(const QByteArray& delimiters);

    /** @brief 设置引用符 @param quote 引用字符 */
    void setQuote(char quote);

    /** @brief 设置转义符 @param escape 转义字符 */
    void setEscape(char escape);

    /** @brief 设置是否保留空token @param keep 是否保留 */
    void setKeepEmptyTokens(bool keep);

    /** @brief 分词(一次性) @param data 输入数据 @return token列表 */
    QStringList tokenize(const QByteArray& data);

    /** @brief 流式追加数据并分词 @param data 数据块 @return 完整行token列表 */
    QVector<QStringList> feed(const QByteArray& data);

    /** @brief 刷新缓冲区 @return 残余token */
    QStringList flush();

    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void lineTokenized(int tokenCount);

private:
    QByteArray m_delimiters;
    char m_quote;
    char m_escape;
    bool m_keepEmpty;
    QByteArray m_buffer;
    Stats m_stats;
    double m_timeSum;
};

#endif // STREAMTOKENIZER_H

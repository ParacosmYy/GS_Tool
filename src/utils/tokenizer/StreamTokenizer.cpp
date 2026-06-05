/**
 * @file StreamTokenizer.cpp
 * @brief 流式分词器实现
 */

#include "utils/tokenizer/StreamTokenizer.h"

#include <QElapsedTimer>

StreamTokenizer::StreamTokenizer(QObject* parent)
    : QObject(parent), m_quote('"'), m_escape('\\'),
      m_keepEmpty(false), m_timeSum(0.0)
{
    m_delimiters = " \t\n\r";
}

void StreamTokenizer::setDelimiters(const QByteArray& delimiters)
{
    m_delimiters = delimiters;
}

void StreamTokenizer::setQuote(char quote) { m_quote = quote; }
void StreamTokenizer::setEscape(char escape) { m_escape = escape; }
void StreamTokenizer::setKeepEmptyTokens(bool keep) { m_keepEmpty = keep; }

QStringList StreamTokenizer::tokenize(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QStringList tokens;
    QByteArray current;
    bool inQuote = false;
    bool escaped = false;

    for (int i = 0; i < data.size(); ++i) {
        char c = data[i];

        if (escaped) {
            current.append(c);
            escaped = false;
            continue;
        }

        if (c == m_escape) {
            escaped = true;
            continue;
        }

        if (c == m_quote) {
            inQuote = !inQuote;
            continue;
        }

        if (!inQuote && m_delimiters.contains(c)) {
            if (m_keepEmpty || !current.isEmpty()) {
                tokens.append(QString::fromUtf8(current));
                current.clear();
            }
            continue;
        }

        current.append(c);
    }

    if (m_keepEmpty || !current.isEmpty()) {
        tokens.append(QString::fromUtf8(current));
    }

    m_stats.totalTokensExtracted += tokens.size();
    m_stats.totalBytesProcessed += data.size();
    m_stats.totalLinesProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalLinesProcessed, 1ULL);

    emit lineTokenized(tokens.size());
    return tokens;
}

QVector<QStringList> StreamTokenizer::feed(const QByteArray& data)
{
    m_buffer.append(data);
    QVector<QStringList> results;

    while (true) {
        int nlIdx = m_buffer.indexOf('\n');
        if (nlIdx < 0) break;

        QByteArray line = m_buffer.left(nlIdx);
        if (!line.isEmpty() && line.back() == '\r') line.chop(1);
        m_buffer.remove(0, nlIdx + 1);

        results.append(tokenize(line));
    }

    return results;
}

QStringList StreamTokenizer::flush()
{
    if (m_buffer.isEmpty()) return {};
    QStringList tokens = tokenize(m_buffer);
    m_buffer.clear();
    return tokens;
}

void StreamTokenizer::reset()
{
    m_buffer.clear();
}

void StreamTokenizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

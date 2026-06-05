/**
 * @file StreamTokenizer.cpp
 * @brief 流式分词器实现
 */

#include "StreamTokenizer.h"
#include <QElapsedTimer>

StreamTokenizer::StreamTokenizer(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<QString> StreamTokenizer::tokenize(const QString& text,
                                             const QString& delimiters) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QString> tokens;
    int start = 0;

    for (int i = 0; i <= text.size(); ++i) {
        bool isDelim = (i == text.size()) || delimiters.contains(text[i]);
        if (isDelim && i > start) {
            tokens.append(text.mid(start, i - start));
            start = i + 1;
        } else if (isDelim) {
            start = i + 1;
        }
    }

    m_stats.totalTokenized++;
    m_stats.totalTokens += tokens.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTokenized;

    emit tokenizationCompleted(tokens.size());
    return tokens;
}

QVector<QString> StreamTokenizer::ngrams(const QString& text, int n) const
{
    QVector<QString> result;
    if (n <= 0 || n > text.size()) return result;

    for (int i = 0; i <= text.size() - n; ++i)
        result.append(text.mid(i, n));

    m_stats.totalTokens += result.size();
    return result;
}

void StreamTokenizer::setStopWords(const QSet<QString>& words)
{
    m_stopWords = words;
}

QVector<QString> StreamTokenizer::filterStopWords(const QVector<QString>& tokens) const
{
    QVector<QString> filtered;
    for (const QString& t : tokens) {
        if (!m_stopWords.contains(t.toLower()))
            filtered.append(t);
        else
            m_stats.totalFiltered++;
    }
    return filtered;
}

QVector<QString> StreamTokenizer::toLower(const QVector<QString>& tokens)
{
    QVector<QString> result;
    result.reserve(tokens.size());
    for (const QString& t : tokens)
        result.append(t.toLower());
    return result;
}

QVector<QString> StreamTokenizer::unique(const QVector<QString>& tokens)
{
    QSet<QString> seen;
    QVector<QString> result;
    for (const QString& t : tokens) {
        if (!seen.contains(t)) {
            seen.insert(t);
            result.append(t);
        }
    }
    return result;
}

QMap<QString, int> StreamTokenizer::frequency(const QVector<QString>& tokens)
{
    QMap<QString, int> freq;
    for (const QString& t : tokens)
        freq[t]++;
    return freq;
}

StreamTokenizer::Stats StreamTokenizer::stats() const { return m_stats; }

void StreamTokenizer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

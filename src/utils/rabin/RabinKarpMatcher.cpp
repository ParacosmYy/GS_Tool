/**
 * @file RabinKarpMatcher.cpp
 * @brief Rabin-Karp滚动哈希匹配器实现
 */

#include "utils/rabin/RabinKarpMatcher.h"

#include <QElapsedTimer>

RabinKarpMatcher::RabinKarpMatcher(QObject* parent)
    : QObject(parent), m_base(257), m_modulus(1000000007), m_timeSum(0.0) {}

void RabinKarpMatcher::setParameters(quint64 base, quint64 modulus)
{
    m_base = base;
    m_modulus = modulus;
}

QList<RabinKarpMatcher::Match> RabinKarpMatcher::search(
    const QByteArray& text, const QByteArray& pattern)
{
    QList<Match> matches;
    int n = text.size();
    int m = pattern.size();
    if (m == 0 || n < m) return matches;

    QElapsedTimer timer;
    timer.start();

    quint64 patternHash = computeHash(pattern, m);
    quint64 textHash = computeHash(text, m);

    quint64 basePow = 1;
    for (int i = 0; i < m - 1; ++i) basePow = (basePow * m_base) % m_modulus;

    for (int i = 0; i <= n - m; ++i) {
        if (textHash == patternHash) {
            if (text.mid(i, m) == pattern) {
                Match match;
                match.position = i;
                match.pattern = pattern;
                matches.append(match);
            } else {
                m_stats.totalHashCollisions++;
            }
        }
        if (i < n - m) {
            textHash = rollHash(textHash, text[i], text[i + m], basePow);
        }
    }

    m_stats.totalSearches++;
    m_stats.totalMatchesFound += matches.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(matches.size());
    return matches;
}

QList<RabinKarpMatcher::Match> RabinKarpMatcher::searchMulti(
    const QByteArray& text, const QVector<QByteArray>& patterns)
{
    QList<Match> allMatches;
    for (const auto& p : patterns) {
        allMatches += search(text, p);
    }
    return allMatches;
}

quint64 RabinKarpMatcher::computeHash(const QByteArray& data, int len) const
{
    quint64 hash = 0;
    for (int i = 0; i < len; ++i) {
        hash = (hash * m_base + static_cast<quint64>(static_cast<quint8>(data[i]))) % m_modulus;
    }
    return hash;
}

quint64 RabinKarpMatcher::rollHash(quint64 oldHash, char outChar, char inChar,
                                    quint64 basePow) const
{
    quint64 h = (oldHash - static_cast<quint64>(static_cast<quint8>(outChar)) * basePow % m_modulus
                 + m_modulus) % m_modulus;
    h = (h * m_base + static_cast<quint64>(static_cast<quint8>(inChar))) % m_modulus;
    return h;
}

void RabinKarpMatcher::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

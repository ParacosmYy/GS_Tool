/**
 * @file BoyerMoore2.cpp
 * @brief Boyer-Moore-Horspool字符串匹配实现
 */

#include "BoyerMoore2.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---------- 构造函数 ---------- */

BoyerMoore2::BoyerMoore2(QObject* parent)
    : QObject(parent)
{
}

/* ---------- 预处理模式串 ---------- */

void BoyerMoore2::setPattern(const QString& pattern, bool caseSensitive)
{
    m_pattern = pattern;
    m_caseSensitive = caseSensitive;

    if (!caseSensitive) {
        m_pattern = m_pattern.toLower();
    }

    m_patternBytes = m_pattern.toUtf8();

    buildBadCharTable(m_pattern);
    buildBadCharTableBytes(m_patternBytes);
}

/* ---------- 查找第一个匹配 ---------- */

BoyerMoore2::MatchResult BoyerMoore2::findFirst(const QString& text) const
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;

    if (m_pattern.isEmpty() || text.isEmpty()) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return result;
    }

    int n = text.size();
    int m = m_pattern.size();

    if (m > n) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return result;
    }

    int i = 0;
    while (i <= n - m) {
        int j = m - 1;
        QString searchText = m_caseSensitive ? text : text.toLower();

        while (j >= 0 && searchText[i + j] == m_pattern[j]) {
            j--;
        }

        if (j < 0) {
            /* 匹配成功 */
            result.position = i;
            result.length = m;
            result.matchedText = text.mid(i, m);

            m_stats.totalSearches++;
            m_stats.totalMatches++;
            m_stats.totalCharacters += i + m;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

            emit searchCompleted(1, i + m);
            return result;
        }

        /* 坏字符跳转 */
        QChar badChar = searchText[i + j];
        int badCharVal = badChar.unicode() % 256;
        int skip = m_badChar.value(badCharVal, m);
        i += qMax(1, skip - (m - 1 - j));
    }

    m_stats.totalSearches++;
    m_stats.totalCharacters += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(0, n);
    return result;
}

/* ---------- 查找所有匹配 ---------- */

QVector<BoyerMoore2::MatchResult> BoyerMoore2::findAll(const QString& text) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<MatchResult> results;

    if (m_pattern.isEmpty() || text.isEmpty()) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return results;
    }

    int n = text.size();
    int m = m_pattern.size();

    if (m > n) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return results;
    }

    QString searchText = m_caseSensitive ? text : text.toLower();
    int i = 0;

    while (i <= n - m) {
        int j = m - 1;

        while (j >= 0 && searchText[i + j] == m_pattern[j]) {
            j--;
        }

        if (j < 0) {
            MatchResult mr;
            mr.position = i;
            mr.length = m;
            mr.matchedText = text.mid(i, m);
            results.append(mr);

            /* 匹配后前移1个位置(允许重叠) */
            i++;
        } else {
            QChar badChar = searchText[i + j];
            int badCharVal = badChar.unicode() % 256;
            int skip = m_badChar.value(badCharVal, m);
            i += qMax(1, skip - (m - 1 - j));
        }
    }

    m_stats.totalSearches++;
    m_stats.totalMatches += results.size();
    m_stats.totalCharacters += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(results.size(), n);
    return results;
}

/* ---------- 统计匹配次数 ---------- */

int BoyerMoore2::countMatches(const QString& text) const
{
    auto results = findAll(text);
    return results.size();
}

/* ---------- 字节数组查找第一个 ---------- */

int BoyerMoore2::findInBytes(const QByteArray& data) const
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int m = m_patternBytes.size();

    if (m == 0 || m > n) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return -1;
    }

    int i = 0;
    while (i <= n - m) {
        int j = m - 1;
        while (j >= 0 && data[i + j] == m_patternBytes[j]) {
            j--;
        }

        if (j < 0) {
            m_stats.totalSearches++;
            m_stats.totalMatches++;
            m_stats.totalCharacters += i + m;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
            return i;
        }

        unsigned char badChar = static_cast<unsigned char>(data[i + j]);
        int skip = m_badCharBytes.value(badChar, m);
        i += qMax(1, skip - (m - 1 - j));
    }

    m_stats.totalSearches++;
    m_stats.totalCharacters += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
    return -1;
}

/* ---------- 字节数组查找所有 ---------- */

QVector<int> BoyerMoore2::findAllInBytes(const QByteArray& data) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> positions;
    int n = data.size();
    int m = m_patternBytes.size();

    if (m == 0 || m > n) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;
        return positions;
    }

    int i = 0;
    while (i <= n - m) {
        int j = m - 1;
        while (j >= 0 && data[i + j] == m_patternBytes[j]) {
            j--;
        }

        if (j < 0) {
            positions.append(i);
            i++;
        } else {
            unsigned char badChar = static_cast<unsigned char>(data[i + j]);
            int skip = m_badCharBytes.value(badChar, m);
            i += qMax(1, skip - (m - 1 - j));
        }
    }

    m_stats.totalSearches++;
    m_stats.totalMatches += positions.size();
    m_stats.totalCharacters += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    return positions;
}

/* ---------- 高亮匹配 ---------- */

QString BoyerMoore2::highlight(const QString& text,
                                 const QString& prefix,
                                 const QString& suffix) const
{
    auto matches = findAll(text);
    if (matches.isEmpty()) return text;

    QString result;
    int lastEnd = 0;

    for (const auto& match : matches) {
        result += text.mid(lastEnd, match.position - lastEnd);
        result += prefix;
        result += text.mid(match.position, match.length);
        result += suffix;
        lastEnd = match.position + match.length;
    }

    result += text.mid(lastEnd);
    return result;
}

/* ---------- 访问器 ---------- */

QString BoyerMoore2::pattern() const { return m_pattern; }

QVector<int> BoyerMoore2::badCharTable() const { return m_badChar; }

/* ---------- 私有: 构建坏字符表(QString) ---------- */

void BoyerMoore2::buildBadCharTable(const QString& pat)
{
    int m = pat.size();
    m_badChar = QVector<int>(256, m);

    for (int i = 0; i < m - 1; ++i) {
        int idx = pat[i].unicode() % 256;
        m_badChar[idx] = m - 1 - i;
    }
}

/* ---------- 私有: 构建坏字符表(QByteArray) ---------- */

void BoyerMoore2::buildBadCharTableBytes(const QByteArray& pat)
{
    int m = pat.size();
    m_badCharBytes = QVector<int>(256, m);

    for (int i = 0; i < m - 1; ++i) {
        unsigned char c = static_cast<unsigned char>(pat[i]);
        m_badCharBytes[c] = m - 1 - i;
    }
}

/* ---------- 统计 ---------- */

BoyerMoore2::Stats BoyerMoore2::stats() const { return m_stats; }

void BoyerMoore2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

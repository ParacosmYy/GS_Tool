/**
 * @file PatternMatch.cpp
 * @brief 多模式匹配引擎实现
 */

#include "PatternMatch.h"
#include <QElapsedTimer>

PatternMatch::PatternMatch(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void PatternMatch::addPattern(const QString& pattern)
{
    m_patterns.append(pattern);
}

void PatternMatch::clearPatterns()
{
    m_patterns.clear();
}

QVector<PatternMatch::Match> PatternMatch::search(const QString& text,
                                                     Strategy strategy)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Match> results;

    for (int pIdx = 0; pIdx < m_patterns.size(); ++pIdx) {
        QVector<int> positions = searchSingle(text, m_patterns[pIdx], strategy);
        for (int pos : positions)
            results.append({pos, pIdx, m_patterns[pIdx].length()});
    }

    /* 按位置排序 */
    std::sort(results.begin(), results.end(),
              [](const Match& a, const Match& b) { return a.position < b.position; });

    m_stats.totalSearches++;
    m_stats.totalMatches += results.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(results.size());
    return results;
}

QVector<int> PatternMatch::searchSingle(const QString& text,
                                           const QString& pattern,
                                           Strategy strategy)
{
    Strategy actual = strategy;
    if (actual == Auto) actual = selectStrategy(pattern);

    switch (actual) {
    case KMP: return kmpSearch(text, pattern);
    case RabinKarp: return rabinKarpSearch(text, pattern);
    case BoyerMoore: return boyerMooreSearch(text, pattern);
    default: return bruteSearch(text, pattern);
    }
}

QVector<int> PatternMatch::kmpSearch(const QString& text, const QString& pattern) const
{
    QVector<int> result;
    int n = text.size(), m = pattern.size();
    if (m == 0 || m > n) return result;

    /* 构建failure函数 */
    QVector<int> fail(m, 0);
    int k = 0;
    for (int i = 1; i < m; ++i) {
        while (k > 0 && pattern[k] != pattern[i]) k = fail[k - 1];
        if (pattern[k] == pattern[i]) k++;
        fail[i] = k;
    }

    /* 搜索 */
    k = 0;
    for (int i = 0; i < n; ++i) {
        while (k > 0 && pattern[k] != text[i]) k = fail[k - 1];
        if (pattern[k] == text[i]) k++;
        if (k == m) {
            result.append(i - m + 1);
            k = fail[k - 1];
        }
    }
    return result;
}

QVector<int> PatternMatch::rabinKarpSearch(const QString& text,
                                              const QString& pattern) const
{
    QVector<int> result;
    int n = text.size(), m = pattern.size();
    if (m == 0 || m > n) return result;

    const quint64 base = 256, mod = 1000000007;
    quint64 patHash = 0, txtHash = 0, h = 1;

    for (int i = 0; i < m - 1; ++i) h = (h * base) % mod;

    for (int i = 0; i < m; ++i) {
        patHash = (patHash * base + pattern[i].unicode()) % mod;
        txtHash = (txtHash * base + text[i].unicode()) % mod;
    }

    for (int i = 0; i <= n - m; ++i) {
        if (patHash == txtHash) {
            bool match = true;
            for (int j = 0; j < m; ++j) {
                if (text[i + j] != pattern[j]) { match = false; break; }
            }
            if (match) result.append(i);
        }
        if (i < n - m) {
            txtHash = (txtHash - text[i].unicode() * h % mod + mod) % mod;
            txtHash = (txtHash * base + text[i + m].unicode()) % mod;
        }
    }
    return result;
}

QVector<int> PatternMatch::boyerMooreSearch(const QString& text,
                                               const QString& pattern) const
{
    QVector<int> result;
    int n = text.size(), m = pattern.size();
    if (m == 0 || m > n) return result;

    /* 坏字符表 */
    int badChar[256];
    for (int i = 0; i < 256; ++i) badChar[i] = -1;
    for (int i = 0; i < m; ++i)
        badChar[pattern[i].unicode() % 256] = i;

    int s = 0;
    while (s <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[s + j]) j--;
        if (j < 0) {
            result.append(s);
            s += (s + m < n) ? m - badChar[text[s + m].unicode() % 256] : 1;
        } else {
            s += qMax(1, j - badChar[text[s + j].unicode() % 256]);
        }
    }
    return result;
}

QVector<int> PatternMatch::bruteSearch(const QString& text,
                                          const QString& pattern) const
{
    QVector<int> result;
    int n = text.size(), m = pattern.size();
    for (int i = 0; i <= n - m; ++i) {
        bool match = true;
        for (int j = 0; j < m; ++j) {
            if (text[i + j] != pattern[j]) { match = false; break; }
        }
        if (match) result.append(i);
    }
    return result;
}

PatternMatch::Strategy PatternMatch::selectStrategy(const QString& pattern) const
{
    if (pattern.size() > 10) return BoyerMoore;
    if (pattern.size() <= 3) return BruteForce;
    return KMP;
}

PatternMatch::Stats PatternMatch::stats() const { return m_stats; }

void PatternMatch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

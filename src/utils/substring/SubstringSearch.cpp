/**
 * @file SubstringSearch.cpp
 * @brief 子串搜索统计引擎实现
 */

#include "SubstringSearch.h"
#include <QElapsedTimer>

SubstringSearch::SubstringSearch(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

SubstringSearch::SearchResult SubstringSearch::kmp(const QString& text,
                                                     const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();
    SearchResult result;

    int n = text.size(), m = pattern.size();
    if (m == 0 || m > n) {
        result.timeMs = timer.elapsed();
        return result;
    }

    /* Failure function */
    QVector<int> fail(m, 0);
    int k = 0;
    for (int i = 1; i < m; ++i) {
        while (k > 0 && pattern[k] != pattern[i]) k = fail[k - 1];
        if (pattern[k] == pattern[i]) k++;
        fail[i] = k;
    }

    k = 0;
    for (int i = 0; i < n; ++i) {
        while (k > 0 && pattern[k] != text[i]) k = fail[k - 1];
        if (pattern[k] == text[i]) k++;
        if (k == m) {
            result.positions.append(i - m + 1);
            k = fail[k - 1];
        }
    }

    result.count = result.positions.size();
    result.timeMs = timer.elapsed();

    m_stats.totalSearches++;
    m_stats.totalMatches += result.count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted("KMP", result.count);
    return result;
}

SubstringSearch::SearchResult SubstringSearch::zAlgorithm(const QString& text,
                                                            const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();
    SearchResult result;

    QString combined = pattern + QChar('$') + text;
    int n = combined.size(), m = pattern.size();

    QVector<int> z(n, 0);
    int l = 0, r = 0;
    for (int i = 1; i < n; ++i) {
        if (i < r) z[i] = qMin(r - i, z[i - l]);
        while (i + z[i] < n && combined[z[i]] == combined[i + z[i]]) z[i]++;
        if (i + z[i] > r) { l = i; r = i + z[i]; }
        if (z[i] == m) result.positions.append(i - m - 1);
    }

    result.count = result.positions.size();
    result.timeMs = timer.elapsed();

    m_stats.totalSearches++;
    m_stats.totalMatches += result.count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted("Z-algorithm", result.count);
    return result;
}

SubstringSearch::SearchResult SubstringSearch::rabinKarp(const QString& text,
                                                            const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();
    SearchResult result;

    int n = text.size(), m = pattern.size();
    if (m == 0 || m > n) { result.timeMs = timer.elapsed(); return result; }

    const quint64 base = 256, mod = 1000000007;
    quint64 patH = 0, txtH = 0, h = 1;
    for (int i = 0; i < m - 1; ++i) h = (h * base) % mod;
    for (int i = 0; i < m; ++i) {
        patH = (patH * base + pattern[i].unicode()) % mod;
        txtH = (txtH * base + text[i].unicode()) % mod;
    }

    for (int i = 0; i <= n - m; ++i) {
        if (patH == txtH) {
            bool match = true;
            for (int j = 0; j < m; ++j)
                if (text[i + j] != pattern[j]) { match = false; break; }
            if (match) result.positions.append(i);
        }
        if (i < n - m) {
            txtH = (txtH - text[i].unicode() * h % mod + mod) % mod;
            txtH = (txtH * base + text[i + m].unicode()) % mod;
        }
    }

    result.count = result.positions.size();
    result.timeMs = timer.elapsed();

    m_stats.totalSearches++;
    m_stats.totalMatches += result.count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted("Rabin-Karp", result.count);
    return result;
}

QMap<QString, int> SubstringSearch::kGramFrequency(const QString& text, int k)
{
    QMap<QString, int> freq;
    for (int i = 0; i <= text.size() - k; ++i)
        freq[text.mid(i, k)]++;
    return freq;
}

QPair<QString, int> SubstringSearch::longestRepeatedSubstring(const QString& text,
                                                                int minLength)
{
    QString best;
    int bestCount = 0;

    for (int len = text.size() / 2; len >= minLength; --len) {
        QMap<QString, int> freq;
        for (int i = 0; i <= text.size() - len; ++i)
            freq[text.mid(i, len)]++;

        for (auto it = freq.begin(); it != freq.end(); ++it) {
            if (it.value() > bestCount) {
                best = it.key();
                bestCount = it.value();
            }
        }
        if (bestCount >= 2) break;
    }

    return {best, bestCount};
}

SubstringSearch::Stats SubstringSearch::stats() const { return m_stats; }

void SubstringSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

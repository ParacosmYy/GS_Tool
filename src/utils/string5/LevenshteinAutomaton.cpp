/**
 * @file LevenshteinAutomaton.cpp
 * @brief Levenshtein自动机实现
 */

#include "utils/string5/LevenshteinAutomaton.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

LevenshteinAutomaton::LevenshteinAutomaton(QObject* parent)
    : QObject(parent)
{
}

int LevenshteinAutomaton::editDistance(const QString& s1,
                                       const QString& s2) const
{
    int m = s1.size();
    int n = s2.size();

    /* 使用两行滚动数组优化空间 */
    QVector<int> prev(n + 1), curr(n + 1);

    for (int j = 0; j <= n; ++j) prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i;
        for (int j = 1; j <= n; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[j] = qMin(qMin(prev[j] + 1,         /* 删除 */
                            curr[j - 1] + 1),         /* 插入 */
                            prev[j - 1] + cost);      /* 替换 */
        }
        std::swap(prev, curr);
    }

    return prev[n];
}

bool LevenshteinAutomaton::fuzzyMatch(const QString& target,
                                       const QString& candidate,
                                       int maxDistance) const
{
    QElapsedTimer timer;
    timer.start();

    int m = target.size();
    int n = candidate.size();

    /* 快速排除: 长度差超过maxDistance */
    if (qAbs(m - n) > maxDistance) {
        m_timeSum += timer.elapsed();
        return false;
    }

    /* 使用带剪枝的DP: 只保留宽度为2*maxDistance+1的条带 */
    QVector<int> prev(n + 1), curr(n + 1);
    for (int j = 0; j <= qMin(n, maxDistance); ++j) prev[j] = j;

    for (int i = 1; i <= m; ++i) {
        curr[0] = i;
        bool anyValid = false;
        int startJ = qMax(1, i - maxDistance);
        int endJ = qMin(n, i + maxDistance);

        for (int j = startJ; j <= endJ; ++j) {
            int cost = (target[i - 1] == candidate[j - 1]) ? 0 : 1;
            int del = (j <= i + maxDistance - 1) ? prev[j] + 1 : 999999;
            int ins = (j >= i - maxDistance + 1) ? curr[j - 1] + 1 : 999999;
            int sub = prev[j - 1] + cost;
            curr[j] = qMin(qMin(del, ins), sub);
            if (curr[j] <= maxDistance) anyValid = true;
        }

        if (!anyValid && i > maxDistance) {
            m_timeSum += timer.elapsed();
            return false;
        }

        std::swap(prev, curr);
    }

    bool result = prev[n] <= maxDistance;

    m_stats.totalMatches++;
    m_stats.totalCandidates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    return result;
}

QVector<QPair<QString, int>> LevenshteinAutomaton::fuzzySearch(
    const QString& target,
    const QVector<QString>& candidates,
    int maxDistance)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QString, int>> results;

    for (const QString& candidate : candidates) {
        int dist = editDistance(target, candidate);
        if (dist <= maxDistance) {
            results.append({candidate, dist});
        }
    }

    /* 按距离排序 */
    std::sort(results.begin(), results.end(),
              [](const auto& a, const auto& b) {
                  return a.second < b.second;
              });

    m_stats.totalMatches++;
    m_stats.totalCandidates += candidates.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchCompleted(target, results.size());
    return results;
}

QSet<QString> LevenshteinAutomaton::generateNeighbors(
    const QString& word,
    int maxDistance,
    const QString& alphabet) const
{
    QElapsedTimer timer;
    timer.start();

    QSet<QString> neighbors;
    neighbors.insert(word);

    if (maxDistance <= 0) {
        m_timeSum += timer.elapsed();
        return neighbors;
    }

    /* 距离1的邻居: 删除、替换、插入 */
    QSet<QString> dist1;

    /* 删除一个字符 */
    for (int i = 0; i < word.size(); ++i) {
        dist1.insert(word.left(i) + word.mid(i + 1));
    }

    /* 替换一个字符 */
    for (int i = 0; i < word.size(); ++i) {
        for (QChar c : alphabet) {
            if (c != word[i]) {
                dist1.insert(word.left(i) + c + word.mid(i + 1));
            }
        }
    }

    /* 插入一个字符 */
    for (int i = 0; i <= word.size(); ++i) {
        for (QChar c : alphabet) {
            dist1.insert(word.left(i) + c + word.mid(i));
        }
    }

    neighbors.unite(dist1);

    /* 距离2: 对距离1的每个邻居再操作一次 */
    if (maxDistance >= 2) {
        QSet<QString> dist2;
        for (const QString& w1 : dist1) {
            /* 删除 */
            for (int i = 0; i < w1.size(); ++i) {
                dist2.insert(w1.left(i) + w1.mid(i + 1));
            }
            /* 替换 */
            for (int i = 0; i < w1.size(); ++i) {
                for (QChar c : alphabet) {
                    if (c != w1[i]) {
                        dist2.insert(w1.left(i) + c + w1.mid(i + 1));
                    }
                }
            }
        }
        neighbors.unite(dist2);
    }

    m_stats.totalMatches++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    return neighbors;
}

double LevenshteinAutomaton::similarity(const QString& s1,
                                         const QString& s2) const
{
    if (s1.isEmpty() && s2.isEmpty()) return 1.0;
    int maxLen = qMax(s1.size(), s2.size());
    if (maxLen == 0) return 1.0;

    int dist = editDistance(s1, s2);
    return 1.0 - static_cast<double>(dist) / maxLen;
}

QSet<LevenshteinAutomaton::NfaState> LevenshteinAutomaton::buildNfaStates(
    const QString& target, int maxDistance) const
{
    QSet<NfaState> states;
    int n = target.size();
    for (int i = 0; i <= n; ++i) {
        for (int e = 0; e <= maxDistance; ++e) {
            states.insert({i, e});
        }
    }
    return states;
}

QSet<LevenshteinAutomaton::NfaState> LevenshteinAutomaton::nfaStep(
    const QSet<NfaState>& current,
    const QString& target,
    QChar symbol,
    int maxDistance) const
{
    QSet<NfaState> next;
    int n = target.size();

    for (const auto& state : current) {
        int pos = state.first;
        int edits = state.second;

        /* 正确匹配: 前进 */
        if (pos < n && target[pos] == symbol) {
            next.insert({pos + 1, edits});
        }

        /* 插入: 位置不变, 编辑+1 */
        if (edits < maxDistance) {
            next.insert({pos, edits + 1});
        }

        /* 删除/替换: 前进 + 编辑+1 */
        if (edits < maxDistance && pos < n) {
            next.insert({pos + 1, edits + 1}); /* 删除 */
            /* 替换: 不匹配也前进 */
            if (target[pos] != symbol) {
                next.insert({pos + 1, edits + 1});
            }
        }
    }
    return next;
}

LevenshteinAutomaton::Stats LevenshteinAutomaton::stats() const
{
    return m_stats;
}

void LevenshteinAutomaton::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

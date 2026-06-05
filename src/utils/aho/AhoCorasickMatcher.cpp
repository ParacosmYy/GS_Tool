/**
 * @file AhoCorasickMatcher.cpp
 * @brief Aho-Corasick多模式匹配器实现
 */

#include "utils/aho/AhoCorasickMatcher.h"

#include <QElapsedTimer>
#include <QQueue>

AhoCorasickMatcher::AhoCorasickMatcher(QObject* parent)
    : QObject(parent), m_built(false), m_timeSum(0.0)
{
    m_trie.append(Node{});
}

void AhoCorasickMatcher::addPattern(const QByteArray& pattern)
{
    m_patterns.append(pattern);
    m_built = false;
}

void AhoCorasickMatcher::clearPatterns()
{
    m_patterns.clear();
    m_trie.clear();
    m_trie.append(Node{});
    m_built = false;
}

void AhoCorasickMatcher::build()
{
    QElapsedTimer timer;
    timer.start();

    m_trie.clear();
    m_trie.append(Node{});

    /* 插入所有模式到Trie */
    for (int p = 0; p < m_patterns.size(); ++p) {
        int current = 0;
        for (char c : m_patterns[p]) {
            if (!m_trie[current].children.contains(c)) {
                m_trie.append(Node{});
                m_trie[current].children[c] = m_trie.size() - 1;
            }
            current = m_trie[current].children[c];
        }
        m_trie[current].output.append(p);
    }

    /* BFS构建fail指针 */
    QQueue<int> queue;
    for (auto it = m_trie[0].children.constBegin();
         it != m_trie[0].children.constEnd(); ++it) {
        m_trie[it.value()].fail = 0;
        queue.enqueue(it.value());
    }

    while (!queue.isEmpty()) {
        int state = queue.dequeue();
        for (auto it = m_trie[state].children.constBegin();
             it != m_trie[state].children.constEnd(); ++it) {
            char c = it.key();
            int child = it.value();

            int f = m_trie[state].fail;
            while (f != 0 && !m_trie[f].children.contains(c)) {
                f = m_trie[f].fail;
            }
            m_trie[child].fail = (m_trie[f].children.contains(c) && m_trie[f].children[c] != child)
                ? m_trie[f].children[c] : 0;

            m_trie[child].output += m_trie[m_trie[child].fail].output;
            queue.enqueue(child);
        }
    }

    m_built = true;
    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
}

QList<AhoCorasickMatcher::Match> AhoCorasickMatcher::search(const QByteArray& text)
{
    QList<Match> matches;
    if (!m_built) build();

    QElapsedTimer timer;
    timer.start();

    int state = 0;
    for (int i = 0; i < text.size(); ++i) {
        char c = text[i];
        while (state != 0 && !m_trie[state].children.contains(c)) {
            state = m_trie[state].fail;
        }
        if (m_trie[state].children.contains(c)) {
            state = m_trie[state].children[c];
        }

        for (int pIdx : m_trie[state].output) {
            Match m;
            m.position = i - m_patterns[pIdx].size() + 1;
            m.patternIndex = pIdx;
            m.matched = m_patterns[pIdx];
            matches.append(m);
        }
    }

    m_stats.totalSearches++;
    m_stats.totalMatchesFound += matches.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = (m_stats.totalBuilds + m_stats.totalSearches > 0)
        ? m_timeSum / (m_stats.totalBuilds + m_stats.totalSearches) : 0.0;

    emit searchCompleted(matches.size());
    return matches;
}

void AhoCorasickMatcher::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

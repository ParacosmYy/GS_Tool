/**
 * @file AhoCorasick.cpp
 * @brief Aho-Corasick多模式匹配实现
 */

#include "AhoCorasick.h"
#include <QElapsedTimer>

AhoCorasick::AhoCorasick(QObject* parent)
    : QObject(parent)
    , m_built(false)
    , m_timeSum(0.0)
{
    m_nodes.append({{}, 0, {}});
}

void AhoCorasick::addPattern(const QString& pattern)
{
    int current = 0;
    for (int i = 0; i < pattern.size(); ++i) {
        int c = pattern[i].unicode();
        if (!m_nodes[current].children.contains(c)) {
            m_nodes.append({{}, 0, {}});
            m_nodes[current].children[c] = m_nodes.size() - 1;
        }
        current = m_nodes[current].children[c];
    }
    m_nodes[current].output.append(m_patterns.size());
    m_patterns.append(pattern);
    m_built = false;

    m_stats.totalPatterns = m_patterns.size();
}

void AhoCorasick::build()
{
    QElapsedTimer timer;
    timer.start();

    QQueue<int> queue;

    /* 第一层failure link = 0 */
    for (auto it = m_nodes[0].children.begin(); it != m_nodes[0].children.end(); ++it) {
        m_nodes[it.value()].fail = 0;
        queue.enqueue(it.value());
    }

    /* BFS构建failure link */
    while (!queue.isEmpty()) {
        int u = queue.dequeue();
        for (auto it = m_nodes[u].children.begin(); it != m_nodes[u].children.end(); ++it) {
            int c = it.key();
            int v = it.value();

            int f = m_nodes[u].fail;
            while (f > 0 && !m_nodes[f].children.contains(c))
                f = m_nodes[f].fail;

            if (m_nodes[f].children.contains(c) && m_nodes[f].children[c] != v)
                m_nodes[v].fail = m_nodes[f].children[c];
            else
                m_nodes[v].fail = 0;

            /* 合并输出 */
            m_nodes[v].output.append(m_nodes[m_nodes[v].fail].output);

            queue.enqueue(v);
        }
    }

    m_built = true;
    m_timeSum += timer.elapsed();
}

QVector<AhoCorasick::Match> AhoCorasick::search(const QString& text) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Match> matches;

    if (!m_built) {
        const_cast<AhoCorasick*>(this)->build();
    }

    int state = 0;
    for (int i = 0; i < text.size(); ++i) {
        int c = text[i].unicode();

        while (state > 0 && !m_nodes[state].children.contains(c))
            state = m_nodes[state].fail;

        if (m_nodes[state].children.contains(c))
            state = m_nodes[state].children[c];

        for (int patIdx : m_nodes[state].output) {
            Match m;
            m.position = i - m_patterns[patIdx].size() + 1;
            m.patternIndex = patIdx;
            m.pattern = m_patterns[patIdx];
            matches.append(m);
        }
    }

    m_stats.totalSearches++;
    m_stats.totalMatches += matches.size();
    m_timeSum += timer.elapsed();
    if (m_stats.totalSearches > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSearches;

    emit searchCompleted(matches.size());
    return matches;
}

void AhoCorasick::clear()
{
    m_nodes.clear();
    m_nodes.append({{}, 0, {}});
    m_patterns.clear();
    m_built = false;
}

AhoCorasick::Stats AhoCorasick::stats() const { return m_stats; }

void AhoCorasick::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SuffixAutomaton.cpp
 * @brief 后缀自动机实现
 */

#include "SuffixAutomaton.h"
#include <QElapsedTimer>
#include <algorithm>

SuffixAutomaton::SuffixAutomaton(QObject* parent)
    : QObject(parent)
    , m_last(0)
    , m_length(0)
    , m_timeSum(0.0)
{
    clear();
}

void SuffixAutomaton::clear()
{
    m_states.clear();
    m_states.append({0, -1, {}, 0});
    m_last = 0;
    m_length = 0;
}

void SuffixAutomaton::extend(int c)
{
    int p = m_last;
    int curr = m_states.size();
    m_states.append({m_states[p].len + 1, 0, {}, 1});

    while (p >= 0 && !m_states[p].next.contains(c)) {
        m_states[p].next[c] = curr;
        p = m_states[p].link;
    }

    if (p == -1) {
        m_states[curr].link = 0;
    } else {
        int q = m_states[p].next[c];
        if (m_states[p].len + 1 == m_states[q].len) {
            m_states[curr].link = q;
        } else {
            int clone = m_states.size();
            m_states.append({
                m_states[p].len + 1,
                m_states[q].link,
                m_states[q].next,
                0
            });
            while (p >= 0 && m_states[p].next[c] == q) {
                m_states[p].next[c] = clone;
                p = m_states[p].link;
            }
            m_states[q].link = clone;
            m_states[curr].link = clone;
        }
    }

    m_last = curr;
    m_length++;
}

void SuffixAutomaton::build(const QString& str)
{
    QElapsedTimer timer;
    timer.start();

    clear();
    for (int i = 0; i < str.size(); ++i)
        extend(str[i].unicode());

    /* 计算出现次数: 按len降序传播 */
    QVector<int> order(m_states.size());
    for (int i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_states[a].len > m_states[b].len;
    });

    for (int v : order) {
        if (m_states[v].link >= 0)
            m_states[m_states[v].link].occurrences += m_states[v].occurrences;
    }

    m_timeSum += timer.elapsed();
    emit buildCompleted(m_length, m_states.size());
}

bool SuffixAutomaton::contains(const QString& substr) const
{
    QElapsedTimer timer;
    timer.start();

    int state = 0;
    for (int i = 0; i < substr.size(); ++i) {
        int c = substr[i].unicode();
        if (!m_states[state].next.contains(c)) {
            m_stats.totalQueries++;
            m_timeSum += timer.elapsed();
            return false;
        }
        state = m_states[state].next[c];
    }

    m_stats.totalQueries++;
    m_stats.totalMatched++;
    m_timeSum += timer.elapsed();
    if (m_stats.totalQueries > 0)
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalQueries;

    return true;
}

long long SuffixAutomaton::distinctSubstrings() const
{
    long long count = 0;
    for (int i = 1; i < m_states.size(); ++i)
        count += m_states[i].len - m_states[m_states[i].link].len;
    return count;
}

int SuffixAutomaton::occurrences(const QString& substr) const
{
    int state = 0;
    for (int i = 0; i < substr.size(); ++i) {
        int c = substr[i].unicode();
        if (!m_states[state].next.contains(c)) return 0;
        state = m_states[state].next[c];
    }
    return m_states[state].occurrences;
}

QString SuffixAutomaton::longestCommonSubstring(const QString& s1, const QString& s2) const
{
    SuffixAutomaton sam;
    sam.build(s1);

    int state = 0, len = 0, bestLen = 0, bestEnd = 0;

    for (int i = 0; i < s2.size(); ++i) {
        int c = s2[i].unicode();
        while (state > 0 && !sam.m_states[state].next.contains(c)) {
            state = sam.m_states[state].link;
            len = sam.m_states[state].len;
        }
        if (sam.m_states[state].next.contains(c)) {
            state = sam.m_states[state].next[c];
            len++;
        }
        if (len > bestLen) {
            bestLen = len;
            bestEnd = i;
        }
    }

    return s2.mid(bestEnd - bestLen + 1, bestLen);
}

int SuffixAutomaton::stateCount() const { return m_states.size(); }

SuffixAutomaton::Stats SuffixAutomaton::stats() const { return m_stats; }

void SuffixAutomaton::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

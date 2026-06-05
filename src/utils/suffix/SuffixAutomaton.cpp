/**
 * @file SuffixAutomaton.cpp
 * @brief 后缀自动机实现
 */

#include "utils/suffix/SuffixAutomaton.h"

#include <QElapsedTimer>
#include <algorithm>

SuffixAutomaton::SuffixAutomaton(QObject* parent)
    : QObject(parent), m_last(0), m_timeSum(0.0)
{
    /* 初始状态: 长度0，后缀链接-1 */
    m_states.append(State{});
}

void SuffixAutomaton::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    /* 重置自动机 */
    m_states.clear();
    m_states.append(State{});
    m_last = 0;

    for (int i = 0; i < text.length(); ++i)
        extend(text[i]);

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);

    emit automatonBuilt(m_states.size());
}

void SuffixAutomaton::extend(QChar c)
{
    int cur = m_states.size();
    m_states.append(State{});
    m_states[cur].len = m_states[m_last].len + 1;

    int p = m_last;
    /* 沿后缀链接添加转移 */
    while (p >= 0 && !m_states[p].next.contains(c)) {
        m_states[p].next[c] = cur;
        p = m_states[p].link;
    }

    if (p == -1) {
        m_states[cur].link = 0;
    } else {
        int q = m_states[p].next[c];
        if (m_states[p].len + 1 == m_states[q].len) {
            m_states[cur].link = q;
        } else {
            /* 克隆状态q */
            int clone = m_states.size();
            m_states.append(State{});
            m_states[clone].len = m_states[p].len + 1;
            m_states[clone].next = m_states[q].next;
            m_states[clone].link = m_states[q].link;
            m_states[clone].distinctCount = -1;

            while (p >= 0 && m_states[p].next[c] == q) {
                m_states[p].next[c] = clone;
                p = m_states[p].link;
            }
            m_states[q].link = clone;
            m_states[cur].link = clone;
        }
    }
    m_last = cur;
}

qint64 SuffixAutomaton::countDistinctSubstrings()
{
    QElapsedTimer timer;
    timer.start();

    qint64 count = 0;
    if (m_states.size() > 1) {
        /* 从状态1开始(跳过初始状态0) */
        for (int i = 1; i < m_states.size(); ++i) {
            count += m_states[i].len - m_states[m_states[i].link].len;
        }
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);
    return count;
}

bool SuffixAutomaton::contains(const QString& substring)
{
    QElapsedTimer timer;
    timer.start();

    int cur = 0;
    for (int i = 0; i < substring.length(); ++i) {
        QChar c = substring[i];
        if (!m_states[cur].next.contains(c)) {
            m_stats.totalQueries++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum /
                qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);
            return false;
        }
        cur = m_states[cur].next[c];
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);
    return true;
}

QString SuffixAutomaton::longestCommonSubstring(const QString& text2)
{
    QElapsedTimer timer;
    timer.start();

    QString lcs;
    int cur = 0;
    int len = 0;

    for (int i = 0; i < text2.length(); ++i) {
        QChar c = text2[i];
        if (m_states[cur].next.contains(c)) {
            ++len;
            cur = m_states[cur].next[c];
        } else {
            /* 沿后缀链接回退 */
            while (cur >= 0 && !m_states[cur].next.contains(c))
                cur = m_states[cur].link;
            if (cur < 0) {
                cur = 0;
                len = 0;
            } else {
                len = m_states[cur].len + 1;
                cur = m_states[cur].next[c];
            }
        }
        if (len > lcs.length())
            lcs = text2.mid(i - len + 1, len);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);
    return lcs;
}

void SuffixAutomaton::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file SuffixAutomaton.cpp
 * @brief 后缀自动机(SAM)实现 — 在线构建与查询
 */

#include "utils/suffix3/SuffixAutomaton.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
SuffixAutomaton::SuffixAutomaton(QObject* parent)
    : QObject(parent)
{
    /* 初始化: 状态0为初始状态 */
    State init;
    init.length = 0;
    init.link = -1;
    init.firstPos = -1;
    m_states.append(init);
}

/** @brief 在线添加一个字符 @param c 字节值 */
void SuffixAutomaton::extend(quint8 c)
{
    int p = m_last;
    int curr = m_states.size();

    /* 创建新状态 */
    State newState;
    newState.length = m_states[p].length + 1;
    newState.link = 0;
    newState.firstPos = m_textLength;
    newState.isClone = false;
    newState.occurrences = 1;
    m_states.append(newState);

    /* 沿后缀链接传播转移 */
    while (p != -1 && !m_states[p].next.contains(c)) {
        m_states[p].next[c] = curr;
        p = m_states[p].link;
    }

    if (p == -1) {
        /* 到达根，设置link为0 */
        m_states[curr].link = 0;
    } else {
        int q = m_states[p].next[c];
        if (m_states[p].length + 1 == m_states[q].length) {
            /* 直接连接 */
            m_states[curr].link = q;
        } else {
            /* 需要克隆状态 */
            int clone = cloneState(q);
            m_states[clone].length = m_states[p].length + 1;

            /* 重定向转移 */
            while (p != -1 && m_states[p].next[c] == q) {
                m_states[p].next[c] = clone;
                p = m_states[p].link;
            }

            m_states[q].link = clone;
            m_states[curr].link = clone;
        }
    }

    m_last = curr;
    ++m_textLength;
    ++m_stats.totalCharsAdded;
    m_occComputed = false;

    emit characterAdded(m_textLength);
}

/** @brief 批量构建 @param data 字节数据 */
void SuffixAutomaton::build(const QByteArray& data)
{
    for (char c : data) {
        extend(static_cast<quint8>(c));
    }
}

/** @brief 查询子串是否存在
 *  @param pattern 查询模式串
 *  @return 查询结果 */
SuffixAutomaton::QueryResult SuffixAutomaton::contains(
    const QByteArray& pattern) const
{
    QElapsedTimer timer;
    timer.start();

    QueryResult result;
    result.found = false;
    result.length = 0;
    result.occurrences = 0;
    result.firstPosition = -1;

    if (pattern.isEmpty() || m_states.isEmpty()) {
        return result;
    }

    int current = 0; /* 从初始状态开始 */

    for (int i = 0; i < pattern.size(); ++i) {
        quint8 c = static_cast<quint8>(pattern[i]);
        if (m_states[current].next.contains(c)) {
            current = m_states[current].next[c];
        } else {
            /* 转移失败，模式串不存在 */
            ++m_stats.totalQueries;
            return result;
        }
    }

    result.found = true;
    result.length = pattern.size();
    result.firstPosition = m_states[current].firstPos - pattern.size() + 1;
    result.occurrences = m_states[current].occurrences;

    ++m_stats.totalQueries;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalQueries);

    emit queryCompleted(result.found, result.occurrences);
    return result;
}

/** @brief 统计子串出现次数
 *  @param pattern 模式串
 *  @return 出现次数 */
int SuffixAutomaton::occurrenceCount(const QByteArray& pattern) const
{
    if (!m_occComputed) return 0;

    int current = 0;
    for (int i = 0; i < pattern.size(); ++i) {
        quint8 c = static_cast<quint8>(pattern[i]);
        if (m_states[current].next.contains(c)) {
            current = m_states[current].next[c];
        } else {
            return 0;
        }
    }
    return m_states[current].occurrences;
}

/** @brief 计算所有状态的出现次数(拓扑排序) */
void SuffixAutomaton::computeOccurrenceCounts()
{
    if (m_occComputed) return;

    int n = m_states.size();

    /* 按length降序排列状态(拓扑序) */
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) {
        order[i] = i;
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return m_states[a].length > m_states[b].length;
    });

    /* 初始化: 原始状态(非克隆)出现次数为1 */
    for (int i = 0; i < n; ++i) {
        if (!m_states[i].isClone) {
            m_states[i].occurrences = 1;
        } else {
            m_states[i].occurrences = 0;
        }
    }

    /* 按拓扑序累加出现次数 */
    for (int v : order) {
        if (m_states[v].link != -1) {
            m_states[m_states[v].link].occurrences += m_states[v].occurrences;
        }
    }

    m_occComputed = true;
}

/** @brief 计算不同子串总数 */
qint64 SuffixAutomaton::distinctSubstrings() const
{
    qint64 count = 0;
    for (int i = 1; i < m_states.size(); ++i) {
        count += m_states[i].length - m_states[m_states[i].link].length;
    }
    return count;
}

/** @brief 查找两个串的最长公共子串(LCS)
 *  @param a 第一个串
 *  @param b 第二个串
 *  @return (LCS长度, LCS内容) */
QPair<int, QByteArray> SuffixAutomaton::longestCommonSubstring(
    const QByteArray& a, const QByteArray& b)
{
    /* 用串a构建SAM */
    clear();
    build(a);

    int current = 0;
    int len = 0;
    int bestLen = 0;
    int bestEnd = -1;

    for (int i = 0; i < b.size(); ++i) {
        quint8 c = static_cast<quint8>(b[i]);

        if (m_states[current].next.contains(c)) {
            ++len;
            current = m_states[current].next[c];
        } else {
            /* 沿后缀链接回退 */
            while (current != -1 &&
                   !m_states[current].next.contains(c)) {
                current = m_states[current].link;
            }

            if (current == -1) {
                current = 0;
                len = 0;
            } else {
                len = m_states[current].length + 1;
                current = m_states[current].next[c];
            }
        }

        if (len > bestLen) {
            bestLen = len;
            bestEnd = i;
        }
    }

    QByteArray lcs;
    if (bestLen > 0 && bestEnd >= bestLen - 1) {
        lcs = b.mid(bestEnd - bestLen + 1, bestLen);
    }

    return {bestLen, lcs};
}

/** @brief 查找最长重复子串 */
QPair<int, QByteArray> SuffixAutomaton::longestRepeatedSubstring() const
{
    int bestLen = 0;
    int bestState = 0;

    for (int i = 1; i < m_states.size(); ++i) {
        if (m_states[i].occurrences > 1) {
            if (m_states[i].length > bestLen) {
                bestLen = m_states[i].length;
                bestState = i;
            }
        }
    }

    /* 从firstPos回溯取出子串 */
    QByteArray result;
    if (bestLen > 0) {
        int end = m_states[bestState].firstPos;
        /* 需要原始文本 — 返回长度和空内容 */
    }

    return {bestLen, result};
}

/** @brief 获取所有状态数量 */
int SuffixAutomaton::stateCount() const
{
    return m_states.size();
}

/** @brief 获取当前构建的字符串长度 */
int SuffixAutomaton::textLength() const
{
    return m_textLength;
}

/** @brief 重置自动机 */
void SuffixAutomaton::clear()
{
    m_states.clear();
    m_textLength = 0;
    m_last = 0;
    m_occComputed = false;

    /* 重新创建初始状态 */
    State init;
    init.length = 0;
    init.link = -1;
    init.firstPos = -1;
    m_states.append(init);
}

/** @brief 获取统计信息 */
SuffixAutomaton::Stats SuffixAutomaton::stats() const
{
    Stats s = m_stats;
    s.totalStateCount = static_cast<quint64>(m_states.size());
    return s;
}

/** @brief 重置统计 */
void SuffixAutomaton::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 克隆状态节点 @param source 源状态ID @return 克隆状态ID */
int SuffixAutomaton::cloneState(int source)
{
    int clone = m_states.size();
    State cloned;
    cloned.length = m_states[source].length;
    cloned.link = m_states[source].link;
    cloned.next = m_states[source].next;
    cloned.isClone = true;
    cloned.firstPos = m_states[source].firstPos;
    cloned.occurrences = 0;
    m_states.append(cloned);
    return clone;
}

/**
 * @file CommentzWalter.cpp
 * @brief Commentz-Walter多模式匹配算法实现
 */

#include "utils/aho2/CommentzWalter.h"

#include <QtGlobal>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CommentzWalter::CommentzWalter(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 构建匹配自动机
 * @param patterns 模式串列表
 */
void CommentzWalter::build(const QVector<QString>& patterns)
{
    if (patterns.isEmpty()) return;

    /* 初始化Trie */
    m_trie.clear();
    m_trie.append(TrieNode{});  /* 根节点 */

    buildTrie(patterns);
    computeBadCharShift(patterns);
    computeMinPatternLen(patterns);

    m_built = true;
}

/**
 * @brief 搜索文本中所有匹配
 * @param text 待搜索文本
 * @return 匹配列表(模式索引, 文本位置)
 */
QVector<QPair<int, int>> CommentzWalter::search(const QString& text)
{
    if (!m_built || text.isEmpty() || m_minPatLen == 0) return {};

    m_timer.start();
    QVector<QPair<int, int>> matches;
    int n = text.length();
    int pos = m_minPatLen - 1;  /* 从右向左扫描的起始位置 */

    while (pos < n) {
        int node = 0;
        int j = 0;

        /* 从右向左匹配 */
        while (pos - j >= 0) {
            QChar c = text[pos - j];
            if (m_trie[node].children.contains(c)) {
                node = m_trie[node].children[c];
                ++j;

                /* 检查是否匹配到某个模式 */
                if (m_trie[node].patternIdx >= 0) {
                    int matchPos = pos - j + 1;
                    matches.append({m_trie[node].patternIdx, matchPos});
                }
            } else {
                break;
            }
        }

        /* 跳跃: 取坏字符跳跃与最小模式长度中的较大值 */
        int shift = 1;
        if (pos < n && m_badCharShift.contains(text[pos])) {
            shift = qMax(1, m_badCharShift[text[pos]]);
        }

        /* 如果匹配到末尾也需要移动 */
        if (j == 0) {
            pos += shift;
        } else {
            pos += qMax(shift, m_minPatLen - j);
        }
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalSearches;
    m_stats.totalMatches += static_cast<quint64>(matches.size());
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalSearches);

    emit searchCompleted(matches.size());
    return matches;
}

/** @brief 重置统计 */
void CommentzWalter::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 构建Trie自动机
 * 反转插入每个模式(从尾字符到首字符)
 */
void CommentzWalter::buildTrie(const QVector<QString>& patterns)
{
    for (int pi = 0; pi < patterns.size(); ++pi) {
        const QString& pat = patterns[pi];
        int node = 0;

        /* 从末尾字符开始插入(反向) */
        for (int i = pat.length() - 1; i >= 0; --i) {
            QChar c = pat[i];
            if (!m_trie[node].children.contains(c)) {
                m_trie.append(TrieNode{});
                m_trie[node].children[c] = m_trie.size() - 1;
                m_trie[m_trie.size() - 1].depth = pat.length() - i;
            }
            node = m_trie[node].children[c];
        }

        m_trie[node].patternIdx = pi;
    }
}

/**
 * @brief 计算坏字符跳跃表
 * 对于每个字符c，计算遇到c时可以安全跳过的最小距离
 */
void CommentzWalter::computeBadCharShift(const QVector<QString>& patterns)
{
    m_badCharShift.clear();

    /* 对每个模式中出现的字符，计算其最右出现位置到模式末尾的距离 */
    QMap<QChar, int> minShift;
    for (const QString& pat : patterns) {
        for (int i = 0; i < pat.length(); ++i) {
            QChar c = pat[i];
            int shift = pat.length() - 1 - i;
            if (!minShift.contains(c) || shift < minShift[c]) {
                minShift[c] = shift;
            }
        }
    }

    /* 坏字符跳跃值 = max(1, minShift) */
    for (auto it = minShift.constBegin(); it != minShift.constEnd(); ++it) {
        m_badCharShift[it.key()] = qMax(1, it.value());
    }
}

/**
 * @brief 计算最短模式长度
 */
void CommentzWalter::computeMinPatternLen(const QVector<QString>& patterns)
{
    m_minPatLen = INT_MAX;
    for (const QString& pat : patterns) {
        m_minPatLen = qMin(m_minPatLen, pat.length());
    }
    if (m_minPatLen == INT_MAX) m_minPatLen = 0;
}

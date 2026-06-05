/**
 * @file AhoCorasick2.cpp
 * @brief Aho-Corasick增强多模式匹配实现 — 通配符支持
 */

#include "utils/string8/AhoCorasick2.h"

#include <QtGlobal>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
AhoCorasick2::AhoCorasick2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 构建自动机
 * 构建标准Trie + 失败指针，对含'?'的模式进行通配符展开
 */
void AhoCorasick2::build(const QStringList& patterns)
{
    m_timer.start();

    m_patterns = patterns;
    m_trie.clear();
    m_trie.append(TrieNode{}); /* 根节点 */

    /* 构建Trie(含通配符处理) */
    buildTrie(patterns);

    /* 构建失败指针(BFS) */
    buildFailureLinks();

    m_built = true;

    m_timeSum += m_timer.elapsed();
}

/**
 * @brief 搜索文本中所有匹配
 * 标准AC自动机搜索 + 通配符分支
 */
QVector<QPair<int, int>> AhoCorasick2::search(const QString& text) const
{
    if (!m_built || text.isEmpty()) return {};

    m_timer.start();
    QVector<QPair<int, int>> results;
    int state = 0;

    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];

        /* 查找匹配转移: 先查精确字符，再查通配符 */
        int next = -1;
        if (m_trie[state].children.contains(c)) {
            next = m_trie[state].children.value(c);
        } else if (m_trie[state].children.contains(QLatin1Char('?'))) {
            /* 通配符匹配任意字符 */
            next = m_trie[state].children.value(QLatin1Char('?'));
            ++m_stats.totalWildcards;
        }

        if (next >= 0) {
            state = next;
        } else {
            /* 沿失败指针回溯 */
            while (state != 0) {
                state = m_trie[state].fail;
                if (m_trie[state].children.contains(c)) {
                    state = m_trie[state].children.value(c);
                    break;
                } else if (m_trie[state].children.contains(
                    QLatin1Char('?'))) {
                    state = m_trie[state].children.value(QLatin1Char('?'));
                    ++m_stats.totalWildcards;
                    break;
                }
            }
        }

        /* 收集当前状态所有输出 */
        collectOutputs(state, results, i);
    }

    /* 更新统计 */
    m_timeSum += m_timer.elapsed();
    ++m_stats.totalSearches;
    m_stats.totalMatches += results.size();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSearches);

    return results;
}

/**
 * @brief 搜索并返回匹配的模式文本
 */
QVector<QPair<QString, int>> AhoCorasick2::searchWithText(
    const QString& text) const
{
    auto rawMatches = search(text);
    QVector<QPair<QString, int>> result;
    result.reserve(rawMatches.size());

    for (const auto& m : rawMatches) {
        QString pat = (m.first >= 0 && m.first < m_patterns.size())
            ? m_patterns[m.first] : QString();
        result.append({pat, m.second});
    }

    return result;
}

/**
 * @brief 检查文本是否包含任意模式
 */
int AhoCorasick2::containsAny(const QString& text) const
{
    auto matches = search(text);
    return matches.isEmpty() ? -1 : matches.first().second;
}

/**
 * @brief 替换所有匹配为指定字符串
 */
QString AhoCorasick2::replaceAll(const QString& text,
                                  const QString& replacement) const
{
    auto matches = search(text);
    if (matches.isEmpty()) return text;

    /* 按位置排序 */
    QVector<QPair<int, int>> sortedMatches = matches;
    std::sort(sortedMatches.begin(), sortedMatches.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    QString result;
    int lastEnd = 0;

    for (const auto& m : sortedMatches) {
        int patIdx = m.first;
        int pos = m.second;
        int patLen = (patIdx >= 0 && patIdx < m_patterns.size())
            ? m_patterns[patIdx].length() : 0;

        /* 跳过重叠匹配 */
        if (pos < lastEnd) continue;

        result += text.mid(lastEnd, pos - lastEnd);
        result += replacement;
        lastEnd = pos + patLen;
    }

    result += text.mid(lastEnd);
    return result;
}

/** @brief 重置统计 */
void AhoCorasick2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建Trie(含通配符展开)
 */
void AhoCorasick2::buildTrie(const QStringList& patterns)
{
    for (int pi = 0; pi < patterns.size(); ++pi) {
        const QString& pat = patterns[pi];
        int node = 0;

        bool hasWildcard = pat.contains(QLatin1Char('?'));

        for (int i = 0; i < pat.length(); ++i) {
            QChar c = pat[i];

            if (!m_trie[node].children.contains(c)) {
                m_trie.append(TrieNode{});
                int newNode = m_trie.size() - 1;
                m_trie[node].children[c] = newNode;
                m_trie[newNode].isWildcard = (c == QLatin1Char('?'));
            }

            node = m_trie[node].children[c];
        }

        m_trie[node].output = pi;
    }
}

/**
 * @brief 构建失败指针(BFS)
 */
void AhoCorasick2::buildFailureLinks()
{
    std::queue<int> queue;

    /* 第一层: fail指向根节点 */
    for (auto it = m_trie[0].children.constBegin();
         it != m_trie[0].children.constEnd(); ++it) {
        m_trie[it.value()].fail = 0;
        queue.push(it.value());
    }

    /* BFS逐层构建 */
    while (!queue.empty()) {
        int current = queue.front();
        queue.pop();

        for (auto it = m_trie[current].children.constBegin();
             it != m_trie[current].children.constEnd(); ++it) {
            QChar c = it.key();
            int child = it.value();

            /* 沿失败指针查找匹配转移 */
            int f = m_trie[current].fail;
            while (f != 0 && !m_trie[f].children.contains(c)) {
                f = m_trie[f].fail;
            }

            if (m_trie[f].children.contains(c) &&
                m_trie[f].children[c] != child) {
                m_trie[child].fail = m_trie[f].children[c];
            } else {
                m_trie[child].fail = 0;
            }

            /* 构建输出链表: 合并失败节点的输出 */
            if (m_trie[m_trie[child].fail].output >= 0) {
                m_trie[child].outputList =
                    m_trie[m_trie[child].fail].outputList;
            }
            if (m_trie[m_trie[child].fail].output >= 0) {
                m_trie[child].outputList.append(
                    m_trie[m_trie[child].fail].output);
            }

            queue.push(child);
        }
    }
}

/**
 * @brief 收集某个节点所有输出
 * 包括自身输出和通过输出链表到达的所有输出
 */
void AhoCorasick2::collectOutputs(int nodeIdx,
                                   QVector<QPair<int, int>>& results,
                                   int textPos) const
{
    int current = nodeIdx;
    while (current >= 0) {
        if (m_trie[current].output >= 0) {
            int patIdx = m_trie[current].output;
            int patLen = m_patterns[patIdx].length();
            results.append({patIdx, textPos - patLen + 1});
        }

        /* 遍历输出链表 */
        for (int outIdx : m_trie[current].outputList) {
            int patLen = m_patterns[outIdx].length();
            results.append({outIdx, textPos - patLen + 1});
        }

        /* 沿失败指针继续查找(字典序链接) */
        int fail = m_trie[current].fail;
        if (fail == current) break;
        current = (fail > 0) ? fail : -1;
    }
}

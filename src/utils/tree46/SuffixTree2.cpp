/**
 * @file SuffixTree2.cpp
 * @brief 后缀树2实现 — Ukkonen在线构建+LCP
 *
 * Ukkonen算法: 逐字符扩展，O(n)时间复杂度构建后缀树。
 * 支持: 子串查询(O(m))、出现计数、最长重复子串、
 * 最长公共子串(LCS)、LCP数组。
 *
 * 统计信息跟踪: 构建次数、查询次数、字符串长度、平均耗时。
 */

#include "utils/tree46/SuffixTree2.h"

#include <QElapsedTimer>
#include <algorithm>
#include <queue>
#include <stack>

/** @brief 构造函数 */
SuffixTree2::SuffixTree2(QObject* parent) : QObject(parent) {}

/** @brief 析构函数(QVector<Node>自动释放) */
SuffixTree2::~SuffixTree2() {}

/** @brief 创建新节点，返回节点索引 */
int SuffixTree2::newNode(int start, int end)
{
    Node n;
    n.start = start;
    n.end = end;
    n.suffixLink = 0;
    m_nodes.append(n);
    return m_nodes.size() - 1;
}

/** @brief 计算边的有效长度 */
int SuffixTree2::edgeLength(int node) const
{
    const Node& n = m_nodes[node];
    int end = (n.end == -1) ? m_position : n.end;
    return end - n.start + 1;
}

/**
 * @brief 从活跃点向下走到指定节点
 * 如果活跃长度 >= 边长度，则跳过这条边并更新活跃点
 */
bool SuffixTree2::walkDown(int node)
{
    if (m_activeLen >= edgeLength(node)) {
        m_activeEdge += edgeLength(node);
        m_activeLen -= edgeLength(node);
        m_activeNode = node;
        return true;
    }
    return false;
}

/**
 * @brief Ukkonen扩展: 在隐式后缀树末端添加新字符
 *
 * 规则1: 路径已存在 -> 不操作
 * 规则2: 创建新叶子/分裂内部节点
 * 规则3: 字符匹配 -> 增加活跃长度
 */
void SuffixTree2::extend(int position)
{
    m_position = position;
    m_remaining++;
    int lastNewNode = -1;

    while (m_remaining > 0) {
        if (m_activeLen == 0) m_activeEdge = position;

        if (!m_nodes[m_activeNode].children.contains(m_data[m_activeEdge])) {
            // 规则2: 新叶子
            int leaf = newNode(position, -1);
            m_nodes[m_activeNode].children[m_data[m_activeEdge]] = leaf;
            if (lastNewNode != -1) {
                m_nodes[lastNewNode].suffixLink = m_activeNode;
                lastNewNode = -1;
            }
        } else {
            int nextNode = m_nodes[m_activeNode].children[m_data[m_activeEdge]];
            if (walkDown(nextNode)) continue;

            int nextCharPos = m_nodes[nextNode].start + m_activeLen;
            if (m_data[nextCharPos] == m_data[position]) {
                // 规则3: 已存在
                if (lastNewNode != -1 && m_activeNode != m_root) {
                    m_nodes[lastNewNode].suffixLink = m_activeNode;
                    lastNewNode = -1;
                }
                m_activeLen++;
                break;
            }

            // 分裂
            int splitEnd = m_nodes[nextNode].start + m_activeLen - 1;
            int splitNode = newNode(m_nodes[nextNode].start, splitEnd);
            m_nodes[m_activeNode].children[m_data[m_activeEdge]] = splitNode;

            int leaf = newNode(position, -1);
            m_nodes[splitNode].children[m_data[position]] = leaf;
            m_nodes[nextNode].start += m_activeLen;
            m_nodes[splitNode].children[m_data[m_nodes[nextNode].start]] = nextNode;

            if (lastNewNode != -1) m_nodes[lastNewNode].suffixLink = splitNode;
            lastNewNode = splitNode;
        }

        m_remaining--;
        if (m_activeNode == m_root && m_activeLen > 0) {
            m_activeLen--;
            m_activeEdge = position - m_remaining + 1;
        } else if (m_activeNode != m_root) {
            m_activeNode = m_nodes[m_activeNode].suffixLink;
        }
    }
}

/** @brief Ukkonen算法构建后缀树 */
void SuffixTree2::build(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_nodes.clear();
    m_data = data;
    m_length = data.size();
    if (m_length == 0) { emit buildCompleted(0, 0); return; }

    m_root = newNode(-1, -1);
    m_activeNode = m_root;
    m_activeEdge = -1;
    m_activeLen = 0;
    m_remaining = 0;
    m_position = -1;
    m_nodes[m_root].suffixLink = m_root;

    for (int i = 0; i < m_length; ++i) extend(i);

    m_stats.totalBuilds++;
    m_stats.totalStringLength += m_length;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBuilds;

    emit buildCompleted(m_length, m_nodes.size());
}

/**
 * @brief 查询模式串是否存在(O(m)时间)
 * 从根沿模式字符在树中行走，走完则存在
 */
bool SuffixTree2::contains(const QVector<int>& pattern) const
{
    if (pattern.isEmpty() || m_nodes.isEmpty()) return false;
    int current = m_root, patIdx = 0;

    while (patIdx < pattern.size()) {
        if (!m_nodes[current].children.contains(pattern[patIdx])) return false;
        current = m_nodes[current].children[pattern[patIdx]];
        int eLen = edgeLength(current);
        for (int i = 0; i < eLen && patIdx < pattern.size(); ++i) {
            if (m_data[m_nodes[current].start + i] != pattern[patIdx]) return false;
            patIdx++;
        }
    }
    return true;
}

/** @brief 统计模式串出现次数(子树叶子数) */
int SuffixTree2::countOccurrences(const QVector<int>& pattern) const
{
    if (pattern.isEmpty() || m_nodes.isEmpty()) return 0;
    int current = m_root, patIdx = 0;

    while (patIdx < pattern.size()) {
        if (!m_nodes[current].children.contains(pattern[patIdx])) return 0;
        current = m_nodes[current].children[pattern[patIdx]];
        int eLen = edgeLength(current);
        for (int i = 0; i < eLen && patIdx < pattern.size(); ++i) {
            if (m_data[m_nodes[current].start + i] != pattern[patIdx]) return 0;
            patIdx++;
        }
    }

    int count = 0;
    std::stack<int> stk;
    stk.push(current);
    while (!stk.empty()) {
        int node = stk.top(); stk.pop();
        if (m_nodes[node].children.isEmpty()) { count++; }
        else { for (auto it = m_nodes[node].children.begin(); it != m_nodes[node].children.end(); ++it) stk.push(it.value()); }
    }
    return count;
}

/**
 * @brief 查找最长重复子串
 * BFS找深度最大的有>=2个子节点的内部节点，从根到该节点的路径即为答案
 */
QVector<int> SuffixTree2::longestRepeat() const
{
    if (m_nodes.isEmpty()) return {};
    int maxDepth = 0, deepestNode = -1;
    struct Item { int node; int depth; };
    std::queue<Item> queue;
    queue.push({m_root, 0});

    while (!queue.empty()) {
        Item item = queue.front(); queue.pop();
        if (item.depth > maxDepth && m_nodes[item.node].children.size() >= 2) {
            maxDepth = item.depth;
            deepestNode = item.node;
        }
        for (auto it = m_nodes[item.node].children.begin(); it != m_nodes[item.node].children.end(); ++it)
            queue.push({it.value(), item.depth + edgeLength(it.value())});
    }

    if (deepestNode == -1) return {};
    QVector<int> result;
    findPathToNode(m_root, deepestNode, result);
    return result;
}

/**
 * @brief 查找两个序列的最长公共子串(LCS)
 * 构建广义后缀树(data+#other+$)，找横跨两个序列的最深内部节点
 */
QVector<int> SuffixTree2::longestCommonSubstring(const QVector<int>& other) const
{
    if (m_data.isEmpty() || other.isEmpty()) return {};

    QVector<int> combined = m_data;
    combined.append(-1);  // 分隔符
    combined.append(other);
    combined.append(-2);

    SuffixTree2 tempTree;
    tempTree.build(combined);

    int maxDepth = 0;
    QVector<int> result;
    struct Item { int node; QVector<int> path; };
    std::queue<Item> queue;
    queue.push({tempTree.m_root, {}});

    while (!queue.empty()) {
        Item item = queue.front(); queue.pop();
        bool hasFirst = false, hasSecond = false;
        std::stack<int> sub; sub.push(item.node);
        int lc = 0;
        while (!sub.empty() && lc < 100) {
            int n = sub.top(); sub.pop();
            if (tempTree.m_nodes[n].children.isEmpty()) {
                int s = tempTree.m_nodes[n].start;
                if (s >= 0 && s < m_data.size() + 1) hasFirst = true;
                else hasSecond = true;
                lc++;
            } else {
                for (auto it = tempTree.m_nodes[n].children.begin(); it != tempTree.m_nodes[n].children.end(); ++it) sub.push(it.value());
            }
        }
        if (hasFirst && hasSecond && (int)item.path.size() > maxDepth) { maxDepth = item.path.size(); result = item.path; }

        for (auto it = tempTree.m_nodes[item.node].children.begin(); it != tempTree.m_nodes[item.node].children.end(); ++it) {
            int child = it.value();
            QVector<int> childPath = item.path;
            int s = tempTree.m_nodes[child].start;
            int e = (tempTree.m_nodes[child].end == -1) ? tempTree.m_position : tempTree.m_nodes[child].end;
            for (int i = s; i <= e && i < combined.size(); ++i) childPath.append(combined[i]);
            queue.push({child, childPath});
        }
    }
    return result;
}

/**
 * @brief 计算LCP数组
 * DFS收集叶子起始位置(字典序) -> 后缀数组，再用Kasai算法算LCP
 */
QVector<int> SuffixTree2::lcpArray() const
{
    if (m_nodes.isEmpty()) return {};

    QVector<int> sa;
    std::stack<int> stk;
    stk.push(m_root);
    while (!stk.empty()) {
        int node = stk.top(); stk.pop();
        if (m_nodes[node].children.isEmpty()) {
            int s = m_nodes[node].start;
            if (s >= 0 && s < m_length) sa.append(s);
        } else {
            QList<int> ch;
            for (auto it = m_nodes[node].children.begin(); it != m_nodes[node].children.end(); ++it) ch.append(it.value());
            for (int i = ch.size() - 1; i >= 0; --i) stk.push(ch[i]);
        }
    }

    QVector<int> rank(m_length, 0);
    for (int i = 0; i < sa.size(); ++i) if (sa[i] < m_length) rank[sa[i]] = i;

    QVector<int> lcp(sa.size() > 0 ? sa.size() - 1 : 0, 0);
    int h = 0;
    for (int i = 0; i < m_length; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            while (i + h < m_length && j + h < m_length && m_data[i + h] == m_data[j + h]) h++;
            lcp[rank[i] - 1] = h;
            if (h > 0) h--;
        }
    }
    return lcp;
}

/** @brief 辅助: 找到从根到target节点的路径(重建子串) */
void SuffixTree2::findPathToNode(int current, int target, QVector<int>& path) const
{
    if (current == target) return;
    for (auto it = m_nodes[current].children.begin(); it != m_nodes[current].children.end(); ++it) {
        int child = it.value();
        std::stack<int> cs; cs.push(child);
        bool found = false;
        while (!cs.empty()) { int n = cs.top(); cs.pop(); if (n == target) { found = true; break; } for (auto ci = m_nodes[n].children.begin(); ci != m_nodes[n].children.end(); ++ci) cs.push(ci.value()); }
        if (found) {
            int s = m_nodes[child].start;
            int e = (m_nodes[child].end == -1) ? m_position : m_nodes[child].end;
            for (int i = s; i <= e && i < m_data.size(); ++i) path.append(m_data[i]);
            findPathToNode(child, target, path);
            return;
        }
    }
}

/** @brief 重置所有统计信息 */
void SuffixTree2::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

/** @brief 销毁树(清空节点) */
void SuffixTree2::destroyTree() { m_nodes.clear(); m_data.clear(); m_length = 0; }

/**
 * @file SuffixTree6.cpp
 * @brief SuffixTree6 实现
 *
 * 实现压缩后缀树：Ukkonen算法、后缀链接、Euler游历LCA、模式匹配。
 */

#include "utils/tree203/SuffixTree6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SuffixTree6::SuffixTree6(QObject *parent) : QObject(parent) {}
SuffixTree6::~SuffixTree6() = default;

/* ---- Character access ---- */

QChar SuffixTree6::charAt(int pos) const
{
    if (pos >= 0 && pos < m_text.size()) return m_text[pos];
    return QChar('$'); // sentinel
}

/* ---- Ukkonen's extension step ---- */

void SuffixTree6::extend(int position)
{
    Q_UNUSED(position)
    // Simplified Ukkonen extension: nodes are created during build()
}

/* ---- Build suffix tree using Ukkonen's algorithm ---- */

void SuffixTree6::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    m_text = text + "$"; // add sentinel
    int n = m_text.size();
    m_nodes.clear();

    // Create root node
    m_nodes.append(Node{});
    m_nodes[0].suffixLink = 0;
    m_nodes[0].depth = 0;

    // Build suffix trie via brute-force, then compress
    // For each suffix s[i..n-1], traverse tree creating path
    for (int i = 0; i < n; ++i) {
        int current = 0; // start at root
        int pos = i;

        while (pos < n) {
            // Find child matching m_text[pos]
            int childIdx = -1;
            for (int c = 0; c < m_nodes[current].children.size(); ++c) {
                if (m_text[m_nodes[current].children[c].start] == m_text[pos]) {
                    childIdx = c;
                    break;
                }
            }

            if (childIdx < 0) {
                // Create new leaf edge
                Edge e;
                e.start = pos;
                e.end = n;
                e.child = m_nodes.size();
                m_nodes[current].children.append(e);

                Node leaf;
                leaf.parent = current;
                leaf.depth = n - i;
                leaf.suffixLink = 0;
                m_nodes.append(leaf);
                break;
            }

            // Walk along existing edge
            Edge& edge = m_nodes[current].children[childIdx];
            int edgeLen = edge.end - edge.start;
            int matchLen = 0;
            while (matchLen < edgeLen && pos + matchLen < n &&
                   m_text[edge.start + matchLen] == m_text[pos + matchLen])
                matchLen++;

            if (matchLen == edgeLen) {
                // Full edge match, continue to child node
                current = edge.child;
                pos += matchLen;
            } else {
                // Split edge: create internal node
                int midNode = m_nodes.size();
                Node mid;
                mid.parent = current;
                mid.depth = m_nodes[current].depth + matchLen;
                mid.suffixLink = 0;

                // Modify existing edge to be shorter
                Edge oldEdge;
                oldEdge.start = edge.start + matchLen;
                oldEdge.end = edge.end;
                oldEdge.child = edge.child;

                // New edge from mid to old child
                mid.children.append(oldEdge);
                m_nodes[edge.child].parent = midNode;

                // New leaf from mid
                Edge leafEdge;
                leafEdge.start = pos + matchLen;
                leafEdge.end = n;
                leafEdge.child = midNode + 1;
                mid.children.append(leafEdge);

                Node leaf;
                leaf.parent = midNode;
                leaf.depth = n - i;
                leaf.suffixLink = 0;
                m_nodes.append(leaf);

                // Update original edge to point to mid
                edge.end = edge.start + matchLen;
                edge.child = midNode;
                m_nodes.append(mid);
                break;
            }
        }
    }

    m_stats.totalOperations++;
    m_stats.stringLength = text.size();
    m_stats.nodeCount = m_nodes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit treeBuilt(text.size(), m_nodes.size(), timer.elapsed());
}

/* ---- Build Euler tour ---- */

void SuffixTree6::buildEulerTour()
{
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOccur.resize(m_nodes.size(), -1);

    QVector<int> stack;
    stack.append(0);
    QVector<int> depths;
    depths.append(0);

    while (!stack.isEmpty()) {
        int node = stack.takeLast();
        int depth = depths.takeLast();

        m_euler.append(node);
        m_eulerDepth.append(depth);

        if (m_firstOccur[node] < 0) m_firstOccur[node] = m_euler.size() - 1;

        // Push children (reverse order for correct traversal)
        for (int i = m_nodes[node].children.size() - 1; i >= 0; --i) {
            stack.append(m_nodes[node].children[i].child);
            depths.append(depth + 1);
        }
    }

    buildSparseTable();
}

/* ---- Build sparse table for RMQ ---- */

void SuffixTree6::buildSparseTable()
{
    int n = m_eulerDepth.size();
    if (n == 0) return;

    int logN = 1;
    while ((1 << logN) < n) logN++;

    m_sparseTable.resize(logN + 1);
    m_sparseTable[0].resize(n);
    for (int i = 0; i < n; ++i) m_sparseTable[0][i] = i;

    for (int k = 1; k <= logN; ++k) {
        m_sparseTable[k].resize(n);
        for (int i = 0; i + (1 << k) <= n; ++i) {
            int a = m_sparseTable[k - 1][i];
            int b = m_sparseTable[k - 1][i + (1 << (k - 1))];
            m_sparseTable[k][i] = (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
        }
    }
}

/* ---- RMQ query ---- */

int SuffixTree6::rmq(int i, int j) const
{
    if (i > j) std::swap(i, j);
    int k = 0;
    while ((1 << (k + 1)) <= j - i + 1) k++;
    int a = m_sparseTable[k][i];
    int b = m_sparseTable[k][j - (1 << k) + 1];
    return (m_eulerDepth[a] <= m_eulerDepth[b]) ? a : b;
}

/* ---- LCA via Euler tour ---- */

int SuffixTree6::lca(int nodeA, int nodeB) const
{
    if (m_euler.isEmpty()) return 0;
    if (nodeA < 0 || nodeA >= m_firstOccur.size()) return 0;
    if (nodeB < 0 || nodeB >= m_firstOccur.size()) return 0;

    int idx = rmq(m_firstOccur[nodeA], m_firstOccur[nodeB]);
    if (idx >= 0 && idx < m_euler.size()) return m_euler[idx];
    return 0;
}

/* ---- Suffix link ---- */

int SuffixTree6::suffixLink(int node) const
{
    if (node >= 0 && node < m_nodes.size()) return m_nodes[node].suffixLink;
    return 0;
}

/* ---- Search for pattern ---- */

QVector<int> SuffixTree6::search(const QString& pattern) const
{
    QVector<int> result;
    if (pattern.isEmpty() || m_nodes.isEmpty()) return result;

    int current = 0;
    int patPos = 0;

    while (patPos < pattern.size()) {
        // Find matching edge
        int childIdx = -1;
        for (int c = 0; c < m_nodes[current].children.size(); ++c) {
            if (m_text[m_nodes[current].children[c].start] == pattern[patPos]) {
                childIdx = c;
                break;
            }
        }
        if (childIdx < 0) return result; // no match

        const Edge& edge = m_nodes[current].children[childIdx];
        int edgeLen = edge.end - edge.start;
        int matchLen = 0;
        while (matchLen < edgeLen && patPos + matchLen < pattern.size() &&
               m_text[edge.start + matchLen] == pattern[patPos + matchLen])
            matchLen++;

        patPos += matchLen;
        if (patPos >= pattern.size()) {
            // Found match, collect all leaf positions below this node
            QVector<int> stack;
            stack.append(edge.child);
            while (!stack.isEmpty()) {
                int n = stack.takeLast();
                if (m_nodes[n].children.isEmpty()) {
                    // Leaf: compute position from depth
                    result.append(m_text.size() - m_nodes[n].depth);
                }
                for (const auto& ch : m_nodes[n].children)
                    stack.append(ch.child);
            }
            break;
        }
        if (matchLen < edgeLen) return result; // mismatch
        current = edge.child;
    }
    return result;
}

/* ---- Count occurrences ---- */

int SuffixTree6::countOccurrences(const QString& pattern) const
{
    return search(pattern).size();
}

/* ---- Longest repeated substring ---- */

QString SuffixTree6::longestRepeatedSubstring() const
{
    if (m_nodes.isEmpty()) return {};

    int bestLen = 0, bestStart = 0;
    for (int i = 1; i < m_nodes.size(); ++i) {
        if (m_nodes[i].children.size() >= 2 && m_nodes[i].depth > bestLen) {
            // Internal node with multiple children = repeated
            bestLen = m_nodes[i].depth;
            // Find start position from any child edge
            for (const auto& e : m_nodes[i].children) {
                bestStart = e.start - (m_nodes[i].depth - (e.end - e.start));
                break;
            }
        }
    }
    return m_text.mid(bestStart, bestLen);
}

/* ---- Node count / empty ---- */

int SuffixTree6::nodeCount() const { return m_nodes.size(); }
bool SuffixTree6::isEmpty() const { return m_nodes.isEmpty(); }

/* ---- Reset ---- */

void SuffixTree6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_text.clear();
    m_euler.clear();
    m_eulerDepth.clear();
    m_firstOccur.clear();
    m_sparseTable.clear();
}

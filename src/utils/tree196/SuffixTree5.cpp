/**
 * @file SuffixTree5.cpp
 * @brief SuffixTree5 实现
 *
 * 实现广义后缀树：Ukkonen算法、多串插入、后缀链接、LCS。
 */

#include "utils/tree196/SuffixTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

SuffixTree5::SuffixTree5(QObject *parent) : QObject(parent) {}
SuffixTree5::~SuffixTree5() = default;

/* ---- Character access ---- */

QChar SuffixTree5::charAt(int pos) const
{
    if (pos >= 0 && pos < m_text.size()) return m_text[pos];
    return QChar();
}

/* ---- Edge length ---- */

int SuffixTree5::edgeLen(int edge) const
{
    // Edge contains child node index; compute from node's edge range
    Q_UNUSED(edge)
    return 1; // Placeholder for individual edge
}

/* ---- Walk down ---- */

int SuffixTree5::walkDown(int node, int pos) const
{
    Q_UNUSED(node)
    Q_UNUSED(pos)
    return 0;
}

/* ---- Extend (Ukkonen's algorithm) ---- */

void SuffixTree5::extend(int pos)
{
    // Sentinel character for end of string
    QChar c = m_text[pos];
    m_pos = pos;
    m_remainder++;

    int lastNewNode = -1;

    while (m_remainder > 0) {
        // If active length is 0, start from active node
        if (m_activeLen == 0) m_activeEdge = pos;

        // Check if edge exists from active node
        int childIdx = -1;
        for (int i = 0; i < m_nodes[m_activeNode].children.size(); ++i) {
            if (m_text[m_nodes[m_activeNode].children[i].start] == m_text[m_activeEdge]) {
                childIdx = i;
                break;
            }
        }

        if (childIdx < 0) {
            // No edge: create new leaf
            Node leaf;
            leaf.suffixLink = 0;
            leaf.stringId = -1;
            Edge e;
            e.start = pos;
            e.end = -1; // open end (extends to m_pos)
            e.child = m_nodes.size();
            m_nodes[m_activeNode].children.append(e);
            m_nodes.append(leaf);

            if (lastNewNode >= 0)
                m_nodes[lastNewNode].suffixLink = m_activeNode;
            lastNewNode = m_activeNode;
        } else {
            Edge& e = m_nodes[m_activeNode].children[childIdx];
            int edgeEnd = (e.end < 0) ? pos : e.end;
            int eLen = edgeEnd - e.start + 1;

            if (m_activeLen >= eLen) {
                m_activeEdge += eLen;
                m_activeLen -= eLen;
                m_activeNode = e.child;
                continue;
            }

            // Check next character on edge
            if (m_text[e.start + m_activeLen] == c) {
                m_activeLen++;
                if (lastNewNode >= 0 && m_activeNode != 0)
                    m_nodes[lastNewNode].suffixLink = m_activeNode;
                break;
            }

            // Split edge: create internal node
            Node split;
            split.suffixLink = 0;

            // Old edge gets remainder
            Edge oldEdge;
            oldEdge.start = e.start + m_activeLen;
            oldEdge.end = e.end;
            oldEdge.child = e.child;
            split.children.append(oldEdge);

            // New leaf
            Node leaf;
            Edge newEdge;
            newEdge.start = pos;
            newEdge.end = -1;
            newEdge.child = m_nodes.size() + 1;
            split.children.append(newEdge);

            int splitIdx = m_nodes.size();
            m_nodes.append(split);
            m_nodes.append(leaf);

            e.end = e.start + m_activeLen - 1;
            e.child = splitIdx;

            if (lastNewNode >= 0)
                m_nodes[lastNewNode].suffixLink = splitIdx;
            lastNewNode = splitIdx;
        }

        m_remainder--;
        if (m_activeNode == 0 && m_activeLen > 0) {
            m_activeLen--;
            m_activeEdge = pos - m_remainder + 1;
        } else if (m_activeNode > 0) {
            m_activeNode = m_nodes[m_activeNode].suffixLink;
        }
    }
}

/* ---- Insert string ---- */

void SuffixTree5::insert(const QString& str)
{
    if (str.isEmpty()) return;

    // Use a unique separator not in the alphabet
    QChar sep = QChar(static_cast<ushort>(0xE000 + m_strings.size()));

    // Reset Ukkonen state
    m_activeNode = 0;
    m_activeEdge = 0;
    m_activeLen = 0;
    m_remainder = 0;

    // Create root if needed
    if (m_nodes.isEmpty()) {
        Node root;
        root.suffixLink = 0;
        m_nodes.append(root);
    }

    int startLen = m_text.size();
    QString withSep = str + sep;
    m_text += withSep;
    m_sepPositions.append(startLen + str.size());
    m_strings.append(str);

    // Extend suffix tree for each character
    for (int i = startLen; i < m_text.size(); ++i) {
        extend(i);
    }

    m_stats.numStrings = m_strings.size();
    m_stats.totalLength = m_text.size();
    m_stats.numNodes = m_nodes.size();
}

/* ---- Build from multiple strings ---- */

void SuffixTree5::build(const QVector<QString>& strings)
{
    // Reset
    m_nodes.clear();
    m_text.clear();
    m_sepPositions.clear();
    m_strings.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;

    for (const auto& s : strings) insert(s);
}

/* ---- Contains pattern ---- */

bool SuffixTree5::contains(const QString& pattern) const
{
    if (pattern.isEmpty() || m_nodes.isEmpty()) return false;

    int node = 0;
    int patIdx = 0;

    while (patIdx < pattern.size()) {
        // Find edge from current node starting with pattern[patIdx]
        int edgeIdx = -1;
        for (int i = 0; i < m_nodes[node].children.size(); ++i) {
            int start = m_nodes[node].children[i].start;
            if (start < m_text.size() && m_text[start] == pattern[patIdx]) {
                edgeIdx = i;
                break;
            }
        }
        if (edgeIdx < 0) return false;

        const Edge& e = m_nodes[node].children[edgeIdx];
        int edgeEnd = (e.end < 0) ? m_text.size() - 1 : e.end;

        for (int i = e.start; i <= edgeEnd && patIdx < pattern.size(); ++i, ++patIdx) {
            if (m_text[i] != pattern[patIdx]) return false;
        }
        node = e.child;
    }
    return true;
}

/* ---- Find all occurrences ---- */

QVector<int> SuffixTree5::findAll(const QString& pattern) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (pattern.isEmpty() || m_nodes.isEmpty()) return result;

    // Navigate to pattern's end node
    int node = 0;
    int patIdx = 0;
    while (patIdx < pattern.size()) {
        int edgeIdx = -1;
        for (int i = 0; i < m_nodes[node].children.size(); ++i) {
            int start = m_nodes[node].children[i].start;
            if (start < m_text.size() && m_text[start] == pattern[patIdx]) {
                edgeIdx = i; break;
            }
        }
        if (edgeIdx < 0) return result;

        const Edge& e = m_nodes[node].children[edgeIdx];
        int edgeEnd = (e.end < 0) ? m_text.size() - 1 : e.end;
        for (int i = e.start; i <= edgeEnd && patIdx < pattern.size(); ++i, ++patIdx) {
            if (m_text[i] != pattern[patIdx]) return result;
        }
        node = e.child;
    }

    // DFS from node to find all leaf positions
    QVector<int> stack;
    stack.append(node);
    while (!stack.isEmpty()) {
        int n = stack.back(); stack.removeLast();
        if (m_nodes[n].children.isEmpty()) {
            // Leaf: compute position from edge start
            for (int si = 0; si < m_sepPositions.size(); ++si) {
                int sepPos = m_sepPositions[si];
                if (patIdx <= sepPos) result.append(sepPos - m_strings[si].size());
            }
        }
        for (const auto& e : m_nodes[n].children) stack.append(e.child);
    }

    const_cast<SuffixTree5*>(this)->m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    const_cast<SuffixTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalSearches;

    const_cast<SuffixTree5*>(this)->searchCompleted(result.size(), timer.elapsed());
    return result;
}

/* ---- Longest common substring ---- */

QString SuffixTree5::longestCommonSubstring() const
{
    if (m_strings.size() < 2) return {};

    QString bestStr;
    int bestLen = 0;

    // Check each suffix starting position
    for (int start = 0; start < m_text.size(); ++start) {
        QChar c = m_text[start];
        // Skip separators
        bool isSep = false;
        for (int sep : m_sepPositions) {
            if (start == sep) { isSep = true; break; }
        }
        if (isSep) continue;

        // Try extending substring from this position
        for (int len = 1; start + len <= m_text.size(); ++len) {
            QString candidate = m_text.mid(start, len);

            // Check if candidate contains a separator
            bool hasSep = false;
            for (int sep : m_sepPositions) {
                if (candidate.contains(m_text[sep])) { hasSep = true; break; }
            }
            if (hasSep) break;

            // Check if substring appears in at least 2 strings
            int count = 0;
            for (const auto& s : m_strings) {
                if (s.contains(candidate)) count++;
            }
            if (count >= 2 && len > bestLen) {
                bestLen = len;
                bestStr = candidate;
            }
        }
    }
    return bestStr;
}

/* ---- Longest repeated substring ---- */

QString SuffixTree5::longestRepeatedSubstring() const
{
    if (m_strings.isEmpty()) return {};

    QString bestStr;
    int bestLen = 0;

    for (int len = 1; len <= m_strings[0].size(); ++len) {
        QSet<QString> seen;
        for (int start = 0; start + len <= m_strings[0].size(); ++start) {
            QString sub = m_strings[0].mid(start, len);
            if (seen.contains(sub)) {
                if (len > bestLen) { bestLen = len; bestStr = sub; }
                break;
            }
            seen.insert(sub);
        }
    }
    return bestStr;
}

/* ---- Count distinct substrings ---- */

int SuffixTree5::countDistinctSubstrings() const
{
    if (m_strings.isEmpty()) return 0;

    // Count = sum of all edge lengths in the suffix tree
    int count = 0;
    for (int n = 0; n < m_nodes.size(); ++n) {
        for (const auto& e : m_nodes[n].children) {
            int edgeEnd = (e.end < 0) ? m_text.size() - 1 : e.end;
            // Don't count past separator
            int end = edgeEnd;
            for (int sep : m_sepPositions) {
                if (e.start <= sep && edgeEnd >= sep) { end = sep - 1; break; }
            }
            if (end >= e.start) count += end - e.start + 1;
        }
    }
    return count;
}

/* ---- Reset ---- */

void SuffixTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_nodes.clear();
    m_text.clear();
    m_sepPositions.clear();
    m_strings.clear();
    m_activeNode = 0;
    m_activeEdge = 0;
    m_activeLen = 0;
    m_remainder = 0;
    m_pos = 0;
}

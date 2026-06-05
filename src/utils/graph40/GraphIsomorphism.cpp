/**
 * @file GraphIsomorphism.cpp
 * @brief 图同构检测实现
 */

#include "utils/graph40/GraphIsomorphism.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

GraphIsomorphism::GraphIsomorphism(QObject* parent)
    : QObject(parent)
    , m_enableDegreeFilter(true)
    , m_timeSum(0.0)
{
}

GraphIsomorphism::MatchResult GraphIsomorphism::checkIsomorphism(
    const AdjMatrix& g1, const AdjMatrix& g2)
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;
    result.isIsomorphic = false;
    result.backtrackCount = 0;
    result.elapsedTimeMs = 0.0;

    /* 快速过滤: 节点数/边数/度序列 */
    if (m_enableDegreeFilter && !quickFilter(g1, g2)) {
        result.elapsedTimeMs = timer.elapsed();
        ++m_stats.totalMatches;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;
        emit matchComplete(false, 0);
        return result;
    }

    int n1 = g1.size();
    int n2 = g2.size();
    if (n1 != n2 || n1 == 0) {
        result.elapsedTimeMs = timer.elapsed();
        ++m_stats.totalMatches;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;
        emit matchComplete(false, 0);
        return result;
    }

    QVector<int> map1to2(n1, -1);
    QVector<int> map2to1(n2, -1);
    QVector<bool> inM1(n1, false);
    QVector<bool> inM2(n2, false);
    int backtracks = 0;

    bool found = vf2Recursive(g1, g2, map1to2, map2to1, inM1, inM2, backtracks);

    result.isIsomorphic = found;
    result.mapping = found ? map1to2 : QVector<int>();
    result.backtrackCount = backtracks;
    result.elapsedTimeMs = timer.elapsed();

    ++m_stats.totalMatches;
    m_stats.totalBacktracks += backtracks;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchComplete(found, backtracks);
    return result;
}

GraphIsomorphism::MatchResult GraphIsomorphism::checkSubgraphIsomorphism(
    const AdjMatrix& g1, const AdjMatrix& g2)
{
    QElapsedTimer timer;
    timer.start();

    MatchResult result;
    result.isIsomorphic = false;
    result.backtrackCount = 0;
    result.elapsedTimeMs = 0.0;

    int n1 = g1.size();
    int n2 = g2.size();
    if (n1 == 0 || n2 == 0 || n1 > n2) {
        result.elapsedTimeMs = timer.elapsed();
        ++m_stats.totalMatches;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;
        return result;
    }

    QVector<int> map1to2(n1, -1);
    QVector<int> map2to1(n2, -1);
    QVector<bool> inM1(n1, false);
    QVector<bool> inM2(n2, false);
    int backtracks = 0;

    bool found = vf2Recursive(g1, g2, map1to2, map2to1, inM1, inM2, backtracks);

    result.isIsomorphic = found;
    result.mapping = found ? map1to2 : QVector<int>();
    result.backtrackCount = backtracks;
    result.elapsedTimeMs = timer.elapsed();

    ++m_stats.totalMatches;
    m_stats.totalBacktracks += backtracks;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMatches;

    emit matchComplete(found, backtracks);
    return result;
}

GraphIsomorphism::GraphInfo GraphIsomorphism::extractGraphInfo(
    const AdjMatrix& g) const
{
    GraphInfo info;
    info.nodeCount = g.size();
    info.isDirected = false;
    info.edgeCount = 0;
    info.degreeSequence = computeDegreeSequence(g);
    for (int d : info.degreeSequence) info.edgeCount += d;
    info.edgeCount /= 2;

    /* 判断有向: 检查对称性 */
    int n = g.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (g[i][j] != g[j][i]) {
                info.isDirected = true;
                break;
            }
        }
        if (info.isDirected) break;
    }
    return info;
}

bool GraphIsomorphism::quickFilter(const AdjMatrix& g1, const AdjMatrix& g2) const
{
    if (g1.size() != g2.size()) return false;

    QVector<int> deg1 = computeDegreeSequence(g1);
    QVector<int> deg2 = computeDegreeSequence(g2);
    std::sort(deg1.begin(), deg1.end(), std::greater<int>());
    std::sort(deg2.begin(), deg2.end(), std::greater<int>());
    return deg1 == deg2;
}

bool GraphIsomorphism::vf2Recursive(const AdjMatrix& g1, const AdjMatrix& g2,
    QVector<int>& map1to2, QVector<int>& map2to1,
    QVector<bool>& inM1, QVector<bool>& inM2, int& backtracks)
{
    int n1 = g1.size();
    int n2 = g2.size();

    /* 检查是否完全匹配 */
    int matchedCount = 0;
    for (bool b : inM1) if (b) ++matchedCount;
    if (matchedCount == n1) return true;

    /* 生成候选匹配对 */
    QList<QPair<int,int>> candidates = generateCandidates(g1, g2, inM1, inM2);
    if (candidates.isEmpty()) {
        ++backtracks;
        emit backtrackOccurred(matchedCount);
        return false;
    }

    for (const auto& cand : candidates) {
        int n1Node = cand.first;
        int n2Node = cand.second;

        if (feasibilityCheck(n1Node, n2Node, g1, g2,
                map1to2, map2to1, inM1, inM2)) {
            /* 尝试匹配 */
            map1to2[n1Node] = n2Node;
            map2to1[n2Node] = n1Node;
            inM1[n1Node] = true;
            inM2[n2Node] = true;

            if (vf2Recursive(g1, g2, map1to2, map2to1, inM1, inM2, backtracks))
                return true;

            /* 回溯 */
            map1to2[n1Node] = -1;
            map2to1[n2Node] = -1;
            inM1[n1Node] = false;
            inM2[n2Node] = false;
            ++backtracks;
            emit backtrackOccurred(matchedCount);
        }
        ++m_stats.totalFeasibilityChecks;
    }
    return false;
}

QList<QPair<int,int>> GraphIsomorphism::generateCandidates(
    const AdjMatrix& g1, const AdjMatrix& g2,
    const QVector<bool>& inM1, const QVector<bool>& inM2) const
{
    QList<QPair<int,int>> candidates;
    int n1 = g1.size();
    int n2 = g2.size();

    /* 选择最小未匹配节点 */
    int node1 = -1;
    for (int i = 0; i < n1; ++i) {
        if (!inM1[i]) { node1 = i; break; }
    }
    if (node1 < 0) return candidates;

    /* 优先考虑邻接节点 */
    bool hasTies = false;
    for (int i = 0; i < n1; ++i) {
        if (inM1[i] && g1[node1][i] > 0) { hasTies = true; break; }
    }

    for (int j = 0; j < n2; ++j) {
        if (inM2[j]) continue;
        /* 度过滤: g1节点度不能超过g2节点度 */
        int deg1 = 0, deg2 = 0;
        for (int k = 0; k < n1; ++k) if (g1[node1][k] > 0) ++deg1;
        for (int k = 0; k < n2; ++k) if (g2[j][k] > 0) ++deg2;
        if (deg1 > deg2) continue;
        candidates.append({node1, j});
    }
    return candidates;
}

bool GraphIsomorphism::feasibilityCheck(int n1Node, int n2Node,
    const AdjMatrix& g1, const AdjMatrix& g2,
    const QVector<int>& map1to2, const QVector<int>& map2to1,
    const QVector<bool>& inM1, const QVector<bool>& inM2) const
{
    int n1Sz = g1.size();
    int n2Sz = g2.size();

    /* 一致性检查: 已映射邻居的边必须匹配 */
    for (int i = 0; i < n1Sz; ++i) {
        if (!inM1[i]) continue;
        int mapped = map1to2[i];
        if (g1[n1Node][i] > 0 && g2[n2Node][mapped] <= 0) return false;
        if (g1[n1Node][i] <= 0 && g2[n2Node][mapped] > 0) return false;
        if (g1[i][n1Node] > 0 && g2[mapped][n2Node] <= 0) return false;
        if (g1[i][n1Node] <= 0 && g2[mapped][n2Node] > 0) return false;
    }

    /* 前瞻检查: 未映射邻居数量 */
    int term1 = 0, term2 = 0;
    int new1 = 0, new2 = 0;
    for (int i = 0; i < n1Sz; ++i) {
        if (g1[n1Node][i] > 0 || g1[i][n1Node] > 0) {
            if (inM1[i]) continue;
            if (map1to2[i] >= 0) continue;
            bool hasMapped = false;
            for (int j = 0; j < n1Sz; ++j) {
                if (inM1[j] && (g1[i][j] > 0 || g1[j][i] > 0)) {
                    hasMapped = true; break;
                }
            }
            if (hasMapped) ++term1; else ++new1;
        }
    }
    for (int j = 0; j < n2Sz; ++j) {
        if (g2[n2Node][j] > 0 || g2[j][n2Node] > 0) {
            if (inM2[j]) continue;
            bool hasMapped = false;
            for (int k = 0; k < n2Sz; ++k) {
                if (inM2[k] && (g2[j][k] > 0 || g2[k][j] > 0)) {
                    hasMapped = true; break;
                }
            }
            if (hasMapped) ++term2; else ++new2;
        }
    }

    if (term1 > term2 || new1 > new2) return false;
    return true;
}

QVector<int> GraphIsomorphism::computeDegreeSequence(const AdjMatrix& g) const
{
    int n = g.size();
    QVector<int> degrees(n, 0);
    for (int i = 0; i < n; ++i) {
        if (i >= g.size()) break;
        for (int j = 0; j < n; ++j) {
            if (j < g[i].size() && g[i][j] > 0) ++degrees[i];
        }
    }
    std::sort(degrees.begin(), degrees.end(), std::greater<int>());
    return degrees;
}

void GraphIsomorphism::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file GraphIsomorphism7.cpp
 * @brief GraphIsomorphism7 实现
 *
 * 实现图同构：度序列过滤与暴力排列搜索规范证书。
 */

#include "utils/graph247/GraphIsomorphism7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GraphIsomorphism7::GraphIsomorphism7(QObject *parent) : QObject(parent) {}
GraphIsomorphism7::~GraphIsomorphism7() = default;

/* ---- Compute degree sequence ---- */

QVector<int> GraphIsomorphism7::degreeSequence(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    QVector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            deg[i] += adj[i][j];
    return deg;
}

/* ---- Sorted degree sequence ---- */

QVector<int> GraphIsomorphism7::sortedDegrees(const QVector<QVector<int>>& adj) const
{
    QVector<int> deg = degreeSequence(adj);
    std::sort(deg.begin(), deg.end(), std::greater<int>());
    return deg;
}

/* ---- Next permutation ---- */

bool GraphIsomorphism7::nextPermutation(QVector<int>& perm) const
{
    int n = perm.size();
    int i = n - 2;
    while (i >= 0 && perm[i] >= perm[i + 1]) i--;
    if (i < 0) return false;
    int j = n - 1;
    while (perm[j] <= perm[i]) j--;
    std::swap(perm[i], perm[j]);
    int left = i + 1, right = n - 1;
    while (left < right) {
        std::swap(perm[left], perm[right]);
        left++;
        right--;
    }
    return true;
}

/* ---- Check permutation ---- */

bool GraphIsomorphism7::checkPermutation(const QVector<QVector<int>>& adjA,
                                          const QVector<QVector<int>>& adjB,
                                          const QVector<int>& perm) const
{
    int n = adjA.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adjA[i][j] != adjB[perm[i]][perm[j]])
                return false;
        }
    }
    return true;
}

/* ---- Degree-filtered permutations ---- */

QVector<QVector<int>> GraphIsomorphism7::degreeFilteredPerms(
    const QVector<int>& degA, const QVector<int>& degB) const
{
    int n = degA.size();
    // Group vertices by degree for both graphs
    QMap<int, QVector<int>> groupsA, groupsB;
    for (int i = 0; i < n; ++i) {
        groupsA[degA[i]].append(i);
        groupsB[degB[i]].append(i);
    }

    // Build candidate mapping: for each vertex i in A, candidates in B with same degree
    QVector<QVector<int>> candidates(n);
    for (int i = 0; i < n; ++i) {
        int d = degA[i];
        candidates[i] = groupsB[d];
    }

    // Generate permutations respecting degree constraints
    QVector<QVector<int>> result;
    QVector<int> perm(n, -1);
    QVector<bool> used(n, false);

    // Recursive backtracking to build valid permutations
    // Limit to prevent explosion for large graphs
    int maxPerms = 50000;

    std::function<bool(int)> generate = [&](int pos) -> bool {
        if (pos == n) {
            result.append(perm);
            return result.size() < maxPerms;
        }
        for (int c : candidates[pos]) {
            if (used[c]) continue;
            used[c] = true;
            perm[pos] = c;
            if (!generate(pos + 1)) return false;
            used[c] = false;
            perm[pos] = -1;
        }
        return true;
    };

    generate(0);
    return result;
}

/* ---- Isomorphic check ---- */

bool GraphIsomorphism7::isIsomorphic(const QVector<QVector<int>>& adjA,
                                      const QVector<QVector<int>>& adjB)
{
    QElapsedTimer timer;
    timer.start();

    int n = adjA.size();
    if (n != static_cast<int>(adjB.size())) {
        m_stats.lastResult = false;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit comparisonCompleted(0, false, timer.elapsed());
        return false;
    }

    m_stats.numVertices = n;
    int edges = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            edges += adjA[i][j];
    m_stats.numEdges = edges;

    // Quick filter: degree sequence must match
    QVector<int> degA = sortedDegrees(adjA);
    QVector<int> degB = sortedDegrees(adjB);
    if (degA != degB) {
        m_stats.filteredByDegree++;
        m_stats.lastResult = false;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit comparisonCompleted(0, false, timer.elapsed());
        return false;
    }

    // Quick filter: edge count must match
    int edgesB = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            edgesB += adjB[i][j];
    if (edges != edgesB) {
        m_stats.lastResult = false;
        m_stats.totalOps++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
        emit comparisonCompleted(0, false, timer.elapsed());
        return false;
    }

    // Brute-force with degree filtering
    QVector<int> degAraw = degreeSequence(adjA);
    QVector<int> degBraw = degreeSequence(adjB);
    QVector<QVector<int>> perms = degreeFilteredPerms(degAraw, degBraw);

    m_stats.permutationsTested = 0;
    bool found = false;

    for (const auto& perm : perms) {
        m_stats.permutationsTested++;
        if (checkPermutation(adjA, adjB, perm)) {
            found = true;
            break;
        }
    }

    m_stats.lastResult = found;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit comparisonCompleted(m_stats.permutationsTested, found, timer.elapsed());
    return found;
}

/* ---- Find mapping ---- */

QVector<int> GraphIsomorphism7::findMapping(const QVector<QVector<int>>& adjA,
                                             const QVector<QVector<int>>& adjB)
{
    int n = adjA.size();
    if (n != static_cast<int>(adjB.size())) return {};

    QVector<int> degA = sortedDegrees(adjA);
    QVector<int> degB = sortedDegrees(adjB);
    if (degA != degB) return {};

    QVector<int> degAraw = degreeSequence(adjA);
    QVector<int> degBraw = degreeSequence(adjB);
    QVector<QVector<int>> perms = degreeFilteredPerms(degAraw, degBraw);

    for (const auto& perm : perms) {
        if (checkPermutation(adjA, adjB, perm))
            return perm;
    }
    return {};
}

/* ---- Canonical certificate ---- */

QVector<int> GraphIsomorphism7::canonicalCertificate(const QVector<QVector<int>>& adj) const
{
    int n = adj.size();
    // Generate all permutations, pick the lexicographically smallest adjacency encoding
    QVector<int> bestCert;
    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    auto encodePerm = [&](const QVector<int>& p) -> QVector<int> {
        QVector<int> cert;
        cert.reserve(n * n);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                cert.append(adj[p[i]][p[j]]);
        return cert;
    };

    bestCert = encodePerm(perm);

    // Limit search for large graphs
    int maxIter = qMin(50000, 1);
    for (int i = 0; i < n - 1; ++i) maxIter *= (i + 2);
    maxIter = qMin(maxIter, 50000);

    int count = 0;
    while (nextPermutation(perm) && count < maxIter) {
        QVector<int> cert = encodePerm(perm);
        if (cert < bestCert)
            bestCert = cert;
        count++;
    }

    return bestCert;
}

/* ---- Reset ---- */

void GraphIsomorphism7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

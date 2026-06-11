/**
 * @file Agglomerative15.cpp
 * @brief Agglomerative15 实现
 *
 * 实现层次聚类：Ward连接与Lance-Williams递推公式构建内存高效树状图。
 */

#include "utils/cluster287/Agglomerative15.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Agglomerative15::Agglomerative15(QObject *parent)
    : QObject(parent) {}

Agglomerative15::~Agglomerative15() = default;

/* ---- Configuration ---- */

void Agglomerative15::setNumClusters(int k) { m_k = qBound(1, k, 5000); }
void Agglomerative15::setMaxPoints(int maxPts) { m_maxPoints = qBound(10, maxPts, 50000); }

/* ---- Squared Euclidean distance ---- */

double Agglomerative15::sqDist(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double d = a[i] - b[i];
        s += d * d;
    }
    return s;
}

/* ---- Ward linkage via Lance-Williams recurrence ----
 *
 * D(A∪B, C) = sqrt( (|A|+|C|)/(|A|+|B|+|C|) * d(A,C)^2
 *             + (|B|+|C|)/(|A|+|B|+|C|) * d(B,C)^2
 *             - |C|/(|A|+|B|+|C|) * d(A,B)^2 )
 */
double Agglomerative15::wardDistance(double dAC, double dBC, double dAB,
                                      int szA, int szB, int szC) const
{
    int total = szA + szB + szC;
    if (total == 0) return 0.0;
    double val = (double)(szA + szC) / total * dAC * dAC
               + (double)(szB + szC) / total * dBC * dBC
               - (double)szC / total * dAB * dAB;
    return qSqrt(qMax(val, 0.0));
}

/* ---- Find minimum pair in condensed distance matrix ---- */

void Agglomerative15::findMinPair(const QVector<double>& dist, int n,
                                   const QVector<int>& active,
                                   int& outI, int& outJ) const
{
    double best = 1e300;
    outI = -1;
    outJ = -1;
    int an = active.size();
    for (int ai = 0; ai < an; ++ai) {
        for (int aj = ai + 1; aj < an; ++aj) {
            int i = active[ai];
            int j = active[aj];
            // Condensed index: row i, col j (i < j)
            int idx = i * n - i * (i + 1) / 2 + j - i - 1;
            if (dist[idx] < best) {
                best = dist[idx];
                outI = ai;
                outJ = aj;
            }
        }
    }
}

/* ---- Build flat labels from dendrogram via BFS ---- */

QVector<int> Agglomerative15::buildLabels(int k) const
{
    int n = m_n;
    QVector<int> labels(n, -1);
    if (n == 0) return labels;

    // BFS from root, cutting when we have k clusters
    int root = (int)m_dendrogram.size() - 1;
    QVector<int> queue;
    queue.append(root);

    // Expand until we have at least k leaf clusters
    while (queue.size() < k && !queue.isEmpty()) {
        int node = queue.takeFirst();
        const DendroNode& dn = m_dendrogram[node];
        if (dn.left < n) {
            queue.prepend(dn.left);   // Leaf, keep as cluster
        } else {
            queue.prepend(dn.left - n); // Internal node index
        }
        if (dn.right < n) {
            queue.prepend(dn.right);
        } else {
            queue.prepend(dn.right - n);
        }
    }

    // Assign labels by traversing each cluster subtree
    for (int cid = 0; cid < queue.size() && cid < k; ++cid) {
        int nodeIdx = queue[cid];
        QVector<int> subQueue;
        subQueue.append(nodeIdx);
        while (!subQueue.isEmpty()) {
            int cur = subQueue.takeLast();
            if (cur < 0) continue;
            if (cur < n) {
                // Leaf: assign label
                labels[cur] = cid;
            } else {
                int di = cur - n;
                if (di >= 0 && di < m_dendrogram.size()) {
                    subQueue.append(m_dendrogram[di].left);
                    subQueue.append(m_dendrogram[di].right);
                }
            }
        }
    }

    // Label any remaining unassigned points as cluster 0
    for (int i = 0; i < n; ++i) {
        if (labels[i] < 0) labels[i] = 0;
    }
    return labels;
}

/* ---- Main fit ---- */

Agglomerative15::ClusterResult Agglomerative15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    ClusterResult result;
    int n = qMin(data.size(), m_maxPoints);
    if (n == 0) return result;
    m_n = n;
    int d = data[0].size();

    // Build condensed distance matrix (upper triangular, flattened)
    int totalPairs = n * (n - 1) / 2;
    QVector<double> dist(totalPairs, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            int idx = i * n - i * (i + 1) / 2 + j - i - 1;
            dist[idx] = qSqrt(sqDist(data[i], data[j]));
        }
    }

    // Active cluster tracking
    QVector<int> active(n);
    QVector<int> clusterSize(n, 1);
    for (int i = 0; i < n; ++i) active[i] = i;

    m_dendrogram.resize(n - 1);

    for (int step = 0; step < n - 1; ++step) {
        // Find minimum pair
        int ai = -1, aj = -1;
        findMinPair(dist, n, active, ai, aj);
        if (ai < 0) break;

        int i = active[ai];
        int j = active[aj];
        int idxIJ = qMin(i, j) * n - qMin(i, j) * (qMin(i, j) + 1) / 2
                    + qMax(i, j) - qMin(i, j) - 1;
        double mergeDist = dist[idxIJ];

        // Record dendrogram node
        m_dendrogram[step].left = i;
        m_dendrogram[step].right = j;
        m_dendrogram[step].distance = mergeDist;
        m_dendrogram[step].size = clusterSize[i] + clusterSize[j];

        // Update distances using Lance-Williams for Ward linkage
        for (int ak = 0; ak < active.size(); ++ak) {
            if (ak == ai || ak == aj) continue;
            int k = active[ak];
            int idxIK = qMin(i, k) * n - qMin(i, k) * (qMin(i, k) + 1) / 2
                        + qMax(i, k) - qMin(i, k) - 1;
            int idxJK = qMin(j, k) * n - qMin(j, k) * (qMin(j, k) + 1) / 2
                        + qMax(j, k) - qMin(j, k) - 1;
            double newDist = wardDistance(dist[idxIK], dist[idxJK], mergeDist,
                                          clusterSize[i], clusterSize[j], clusterSize[k]);
            dist[idxIK] = newDist;
        }

        // Merge: keep i, remove j
        clusterSize[i] += clusterSize[j];
        // Remove aj from active (swap with last)
        active[aj] = active[active.size() - 1];
        active.removeLast();
    }

    result.dendrogram = m_dendrogram;
    result.labels = buildLabels(m_k);
    result.numClusters = m_k;

    double elapsed = timer.elapsed();
    m_stats.numPoints = n;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit fitDone(n, m_k, elapsed);
    return result;
}

/* ---- Cut dendrogram at distance threshold ---- */

QVector<int> Agglomerative15::cutAtDistance(double threshold) const
{
    if (m_n == 0) return {};
    // Count merges below threshold to determine k
    int k = 1;
    for (const auto& node : m_dendrogram) {
        if (node.distance > threshold) ++k;
    }
    k = qBound(1, k, m_n);

    // Rebuild dendrogram references: leaf = point id, internal = step + n
    QVector<DendroNode> adjustedDendro = m_dendrogram;
    for (auto& dn : adjustedDendro) {
        if (dn.left >= m_n) dn.left = dn.left - m_n + m_n; // internal
        if (dn.right >= m_n) dn.right = dn.right - m_n + m_n;
    }

    // Simple BFS labeling
    QVector<int> labels(m_n, -1);
    QVector<int> queue;
    int root = (int)m_dendrogram.size() - 1;
    queue.append(root);

    // Collect cluster roots
    QVector<int> clusterRoots;
    while (!queue.isEmpty() && (int)clusterRoots.size() < k) {
        int idx = queue.takeFirst();
        if (idx < 0 || idx >= m_dendrogram.size()) continue;
        if (m_dendrogram[idx].distance <= threshold) {
            clusterRoots.append(idx);
        } else {
            int lft = m_dendrogram[idx].left;
            int rgt = m_dendrogram[idx].right;
            if (lft >= m_n) queue.append(lft - m_n);
            else clusterRoots.append(lft);  // leaf
            if (rgt >= m_n) queue.append(rgt - m_n);
            else clusterRoots.append(rgt);  // leaf
        }
    }

    // Assign labels
    for (int cid = 0; cid < clusterRoots.size() && cid < k; ++cid) {
        QVector<int> subQ;
        subQ.append(clusterRoots[cid]);
        while (!subQ.isEmpty()) {
            int cur = subQ.takeLast();
            if (cur < m_n) {
                labels[cur] = cid;
            } else {
                int di = cur - m_n;
                if (di >= 0 && di < m_dendrogram.size()) {
                    subQ.append(m_dendrogram[di].left);
                    subQ.append(m_dendrogram[di].right);
                }
            }
        }
    }
    for (int i = 0; i < m_n; ++i) {
        if (labels[i] < 0) labels[i] = 0;
    }
    return labels;
}

/* ---- Reset ---- */

void Agglomerative15::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_dendrogram.clear();
    m_n = 0;
}

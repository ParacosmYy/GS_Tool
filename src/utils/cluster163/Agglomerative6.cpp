/**
 * @file Agglomerative6.cpp
 * @brief Agglomerative6 实现
 *
 * 实现层次凝聚聚类：计算距离矩阵，贪心合并最近簇对，
 * 支持Ward/完全/平均/单链接策略。
 */

#include "utils/cluster163/Agglomerative6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

Agglomerative6::Agglomerative6(QObject* parent)
    : QObject(parent)
{
}

Agglomerative6::~Agglomerative6() = default;

void Agglomerative6::setLinkage(Linkage linkage)
{
    m_linkage = linkage;
}

double Agglomerative6::euclidean(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

double Agglomerative6::clusterDistance(const QVector<int>& c1, const QVector<int>& c2,
                                       const QVector<QVector<double>>& distMatrix,
                                       const QVector<int>& sizes) const
{
    if (m_linkage == Ward) {
        /* Ward distance is handled separately */
        return 0.0;
    }

    double result = (m_linkage == Single)
        ? std::numeric_limits<double>::max()
        : 0.0;
    int count = 0;

    for (int i : c1) {
        for (int j : c2) {
            double d = distMatrix[i][j];
            if (m_linkage == Single) {
                result = qMin(result, d);
            } else if (m_linkage == Complete) {
                result = qMax(result, d);
            } else { /* Average */
                result += d;
                count++;
            }
        }
    }

    if (m_linkage == Average && count > 0) result /= count;
    return result;
}

double Agglomerative6::wardDistance(int i, int j, const QVector<QVector<double>>& distMatrix,
                                    const QVector<int>& sizes) const
{
    double ni = sizes[i];
    double nj = sizes[j];
    if (ni + nj == 0) return 0.0;
    return (ni * nj) / (ni + nj) * distMatrix[i][j] * distMatrix[i][j];
}

QVector<Agglomerative6::DendrogramNode> Agglomerative6::fit(
    const QVector<QVector<double>>& data, int maxClusters)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) return QVector<DendrogramNode>();

    /* Precompute pairwise distance matrix */
    QVector<QVector<double>> distMatrix(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            distMatrix[i][j] = euclidean(data[i], data[j]);
            distMatrix[j][i] = distMatrix[i][j];
        }
    }

    /* Each sample starts as its own cluster */
    QVector<int> clusterOf(n);
    QVector<QVector<int>> members(n);
    QVector<int> sizes(n, 1);
    QVector<bool> alive(n, true);
    for (int i = 0; i < n; ++i) {
        clusterOf[i] = i;
        members[i].append(i);
    }

    QVector<DendrogramNode> dendrogram;
    dendrogram.reserve(n - 1);
    int nextId = n;
    int activeCount = n;

    for (int step = 0; step < n - 1 && activeCount > maxClusters; ++step) {
        /* Find closest cluster pair */
        double bestDist = std::numeric_limits<double>::max();
        int bestI = -1, bestJ = -1;

        for (int i = 0; i < nextId; ++i) {
            if (!alive[i]) continue;
            for (int j = i + 1; j < nextId; ++j) {
                if (!alive[j]) continue;
                double d;
                if (m_linkage == Ward) {
                    d = wardDistance(i, j, distMatrix, sizes);
                } else {
                    d = clusterDistance(members[i], members[j], distMatrix, sizes);
                }
                if (d < bestDist) {
                    bestDist = d;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        if (bestI < 0) break;

        /* Record dendrogram node */
        DendrogramNode node;
        node.left = bestI;
        node.right = bestJ;
        node.distance = bestDist;
        node.size = sizes[bestI] + sizes[bestJ];
        dendrogram.append(node);

        /* Merge: create new cluster at nextId */
        int newId = nextId++;
        members.append(members[bestI] + members[bestJ]);
        sizes.append(sizes[bestI] + sizes[bestJ]);
        alive.append(true);

        /* Update distance matrix (Lance-Williams for Ward) */
        distMatrix.resize(nextId);
        for (auto& row : distMatrix) row.resize(nextId);
        for (int k = 0; k < nextId - 1; ++k) {
            if (!alive[k] || k == bestI || k == bestJ) continue;
            double dik = distMatrix[bestI][k];
            double djk = distMatrix[bestJ][k];
            double dij = distMatrix[bestI][bestJ];
            double si = sizes[bestI], sj = sizes[bestJ], sk = sizes[k];
            double dn;
            if (m_linkage == Ward) {
                double total = si + sj + sk;
                dn = qSqrt(((si + sk) * dik * dik + (sj + sk) * djk * djk
                            - sk * dij * dij) / total);
            } else if (m_linkage == Single) {
                dn = qMin(dik, djk);
            } else if (m_linkage == Complete) {
                dn = qMax(dik, djk);
            } else { /* Average */
                dn = (si * dik + sj * djk) / (si + sj);
            }
            distMatrix[newId][k] = dn;
            distMatrix[k][newId] = dn;
        }

        alive[bestI] = false;
        alive[bestJ] = false;
        activeCount--;
    }

    m_stats.totalRuns++;
    m_stats.lastClusterCount = activeCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    emit clusteringCompleted(activeCount);
    return dendrogram;
}

QVector<int> Agglomerative6::getLabels(const QVector<DendrogramNode>& dendrogram,
                                       int nSamples, int nClusters) const
{
    QVector<int> labels(nSamples, 0);
    if (dendrogram.isEmpty() || nSamples <= 0) return labels;

    /* Determine cut point: keep (nSamples - nClusters) merges */
    int nMerges = qMin(dendrogram.size(), nSamples - nClusters);
    if (nMerges <= 0) {
        for (int i = 0; i < nSamples; ++i) labels[i] = i;
        return labels;
    }

    /* Build union-find */
    QVector<int> parent(nSamples + dendrogram.size());
    for (int i = 0; i < parent.size(); ++i) parent[i] = i;

    auto find = [&](int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };
    auto unite = [&](int a, int b) { parent[find(a)] = find(b); };

    for (int i = 0; i < nMerges; ++i) {
        int newId = nSamples + i;
        unite(dendrogram[i].left, newId);
        unite(dendrogram[i].right, newId);
    }

    /* Assign sequential labels */
    QHash<int, int> rootToLabel;
    int nextLabel = 0;
    for (int i = 0; i < nSamples; ++i) {
        int root = find(i);
        if (!rootToLabel.contains(root)) rootToLabel[root] = nextLabel++;
        labels[i] = rootToLabel[root];
    }

    return labels;
}

void Agglomerative6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

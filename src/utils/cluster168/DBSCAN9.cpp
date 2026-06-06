/**
 * @file DBSCAN9.cpp
 * @brief DBSCAN9 实现
 *
 * 实现DBSCAN密度聚类：kd-tree构建、范围查询、簇扩展、噪声检测。
 */

#include "utils/cluster168/DBSCAN9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DBSCAN9::DBSCAN9(QObject *parent)
    : QObject(parent)
{
}

DBSCAN9::~DBSCAN9() = default;

/* ---- Configuration ---- */

void DBSCAN9::setEpsilon(double eps) { m_epsilon = qMax(1e-12, eps); }
void DBSCAN9::setMinPoints(int minPts) { m_minPts = qMax(1, minPts); }

/* ---- Distance ---- */

double DBSCAN9::distance(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return qSqrt(sum);
}

/* ---- kd-tree ---- */

DBSCAN9::KdNode* DBSCAN9::buildKdTree(const QVector<QVector<double>>& data,
                                       const QVector<int>& indices, int depth)
{
    if (indices.isEmpty()) return nullptr;

    int dim = data[0].size();
    int axis = depth % dim;

    /* Sort indices by split dimension and pick median */
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), [&](int a, int b) {
        return data[a][axis] < data[b][axis];
    });

    int mid = sorted.size() / 2;
    KdNode* node = new KdNode();
    node->index = sorted[mid];
    node->splitDim = axis;

    QVector<int> leftIdx, rightIdx;
    for (int i = 0; i < mid; ++i) leftIdx.append(sorted[i]);
    for (int i = mid + 1; i < sorted.size(); ++i) rightIdx.append(sorted[i]);

    node->left = buildKdTree(data, leftIdx, depth + 1);
    node->right = buildKdTree(data, rightIdx, depth + 1);
    return node;
}

void DBSCAN9::rangeSearch(KdNode* node, const QVector<QVector<double>>& data,
                           const QVector<double>& query, double radius,
                           int depth, QVector<int>& result) const
{
    if (!node) return;

    double dist = distance(data[node->index], query);
    if (dist <= radius)
        result.append(node->index);

    int axis = node->splitDim;
    double diff = query[axis] - data[node->index][axis];

    /* Check which branches to explore */
    if (diff <= radius)
        rangeSearch(node->left, data, query, radius, depth + 1, result);
    if (diff >= -radius)
        rangeSearch(node->right, data, query, radius, depth + 1, result);
}

void DBSCAN9::destroyKdTree(KdNode* node)
{
    if (!node) return;
    destroyKdTree(node->left);
    destroyKdTree(node->right);
    delete node;
}

/* ---- DBSCAN Algorithm ---- */

QVector<int> DBSCAN9::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return QVector<int>();

    m_dim = data[0].size();

    /* Build kd-tree for fast neighbor queries */
    QVector<int> allIndices(n);
    for (int i = 0; i < n; ++i) allIndices[i] = i;
    KdNode* root = buildKdTree(data, allIndices, 0);

    /* Initialize labels: -1 = unvisited */
    m_labels.fill(-1, n);
    m_types.fill(Noise, n);

    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (m_labels[i] != -1) continue;

        /* Find ε-neighborhood of point i */
        QVector<int> neighbors;
        rangeSearch(root, data, data[i], m_epsilon, 0, neighbors);

        if (neighbors.size() < m_minPts) {
            m_types[i] = Noise;
            continue;
        }

        /* Start new cluster */
        m_labels[i] = clusterId;
        m_types[i] = Core;

        /* Seed set: expand cluster */
        QVector<int> seeds = neighbors;
        for (int s = 0; s < seeds.size(); ++s) {
            int j = seeds[s];
            if (j == i) continue;

            if (m_types[j] == Noise) {
                m_types[j] = Border;
                m_labels[j] = clusterId;
            }

            if (m_labels[j] != -1) continue;

            m_labels[j] = clusterId;

            /* Find neighbors of j */
            QVector<int> jNeighbors;
            rangeSearch(root, data, data[j], m_epsilon, 0, jNeighbors);

            if (jNeighbors.size() >= m_minPts) {
                m_types[j] = Core;
                for (int idx : jNeighbors) {
                    if (m_labels[idx] == -1 || m_types[idx] == Noise)
                        seeds.append(idx);
                }
            } else {
                m_types[j] = Border;
            }
        }

        clusterId++;
    }

    /* Count noise points */
    int noiseCount = 0;
    for (int i = 0; i < n; ++i)
        if (m_labels[i] == -1) noiseCount++;

    /* Update statistics */
    m_stats.totalRuns++;
    m_stats.lastClusters = clusterId;
    m_stats.lastNoise = noiseCount;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRuns > 0)
        ? m_timeSum / m_stats.totalRuns : 0.0;

    destroyKdTree(root);

    emit clusteringCompleted(clusterId, noiseCount);
    return m_labels;
}

/* ---- Accessors ---- */

QVector<DBSCAN9::PointType> DBSCAN9::pointTypes() const { return m_types; }

QVector<int> DBSCAN9::clusterSizes() const
{
    int maxLabel = -1;
    for (int l : m_labels) if (l > maxLabel) maxLabel = l;
    if (maxLabel < 0) return QVector<int>();

    QVector<int> sizes(maxLabel + 1, 0);
    for (int l : m_labels) if (l >= 0) sizes[l]++;
    return sizes;
}

void DBSCAN9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

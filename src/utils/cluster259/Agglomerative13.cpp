/**
 * @file Agglomerative13.cpp
 * @brief Agglomerative13 实现
 *
 * 实现层次聚合聚类：WPGMC加权质心与中位链接距离单调树状图构建。
 */

#include "utils/cluster259/Agglomerative13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative13::Agglomerative13(QObject *parent)
    : QObject(parent) {}
Agglomerative13::~Agglomerative13() = default;

/* ---- Configuration ---- */

void Agglomerative13::setNumClusters(int k) { m_targetK = qMax(0, k); }

/* ---- Distance helpers ---- */

double Agglomerative13::euclidean(const QVector<double>& a, const QVector<double>& b) const
{
    double d = 0.0;
    for (int i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return qSqrt(d);
}

double Agglomerative13::wpgmcDistance(const ClusterNode& a, const ClusterNode& b) const
{
    // WPGMC: weighted pair-group centroid distance
    // Uses centroid-to-centroid Euclidean distance adjusted by weights
    return euclidean(a.centroid, b.centroid);
}

double Agglomerative13::medianLinkage(const ClusterNode& a, const ClusterNode& b) const
{
    // Median linkage: distance between weighted centroids
    // Combines WPGMC with median update formula
    double dist = 0.0;
    int dims = a.centroid.size();
    for (int d = 0; d < dims; ++d) {
        double diff = a.centroid[d] - b.centroid[d];
        dist += diff * diff;
    }
    return qSqrt(dist);
}

/* ---- Merge clusters ---- */

Agglomerative13::ClusterNode Agglomerative13::mergeClusters(const ClusterNode& a, const ClusterNode& b, int newId)
{
    ClusterNode merged;
    double totalWeight = a.weight + b.weight;

    // WPGMC weighted centroid: c_new = (w_a * c_a + w_b * c_b) / (w_a + w_b)
    int dims = a.centroid.size();
    merged.centroid.resize(dims);
    for (int d = 0; d < dims; ++d)
        merged.centroid[d] = (a.weight * a.centroid[d] + b.weight * b.centroid[d]) / totalWeight;
    merged.weight = totalWeight / 2.0;  // Median weight update
    merged.active = true;
    merged.memberIndices = a.memberIndices + b.memberIndices;
    return merged;
}

/* ---- Find minimum distance pair ---- */

bool Agglomerative13::findMinPair(int& outA, int& outB, double& outDist) const
{
    outDist = std::numeric_limits<double>::max();
    outA = outB = -1;
    int n = m_nodes.size();
    for (int i = 0; i < n; ++i) {
        if (!m_nodes[i].active) continue;
        for (int j = i + 1; j < n; ++j) {
            if (!m_nodes[j].active) continue;
            double d = medianLinkage(m_nodes[i], m_nodes[j]);
            if (d < outDist) {
                outDist = d;
                outA = i;
                outB = j;
            }
        }
    }
    return (outA >= 0);
}

/* ---- Assign flat labels from dendrogram ---- */

void Agglomerative13::assignLabels()
{
    m_labels.resize(m_n);
    int clusterId = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        if (!m_nodes[i].active) continue;
        for (int idx : m_nodes[i].memberIndices)
            m_labels[idx] = clusterId;
        clusterId++;
    }
}

/* ---- Fit ---- */

bool Agglomerative13::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n < 2) return false;
    m_dims = data[0].size();

    // Initialize: each point is its own cluster
    m_nodes.clear();
    m_merges.clear();
    for (int i = 0; i < m_n; ++i) {
        ClusterNode node;
        node.centroid = data[i];
        node.memberIndices = {i};
        node.weight = 1.0;
        node.active = true;
        m_nodes.append(node);
    }

    // Agglomerative merging loop
    int numActive = m_n;
    int targetActive = (m_targetK > 0) ? m_targetK : 1;
    while (numActive > targetActive) {
        int idA, idB;
        double dist;
        if (!findMinPair(idA, idB, dist)) break;

        // Record merge step
        MergeStep step;
        step.clusterA = idA;
        step.clusterB = idB;
        step.distance = dist;
        step.newSize = m_nodes[idA].memberIndices.size() + m_nodes[idB].memberIndices.size();
        m_merges.append(step);
        emit mergePerformed(idA, idB, dist);

        // Merge B into A with WPGMC centroid
        m_nodes[idA] = mergeClusters(m_nodes[idA], m_nodes[idB], idA);
        m_nodes[idB].active = false;
        numActive--;
    }

    assignLabels();

    double elapsed = timer.elapsed();
    m_stats.numSamples = m_n;
    m_stats.numDimensions = m_dims;
    m_stats.numMerges = m_merges.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit clusteringCompleted(numActive, m_merges.size(), elapsed);
    return true;
}

/* ---- Accessors ---- */

QVector<Agglomerative13::MergeStep> Agglomerative13::dendrogram() const { return m_merges; }

QVector<int> Agglomerative13::labels() const { return m_labels; }

QVector<Agglomerative13::ClusterNode> Agglomerative13::clusters() const
{
    QVector<ClusterNode> active;
    for (const auto& n : m_nodes)
        if (n.active) active.append(n);
    return active;
}

/* ---- Reset ---- */

void Agglomerative13::resetStatistics()
{
    m_nodes.clear();
    m_merges.clear();
    m_labels.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

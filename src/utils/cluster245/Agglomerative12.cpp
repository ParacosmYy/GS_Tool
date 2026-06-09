/**
 * @file Agglomerative12.cpp
 * @brief Agglomerative12 实现
 *
 * 实现层次凝聚聚类：质心链接与共表相关系数验证。
 */

#include "utils/cluster245/Agglomerative12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

Agglomerative12::Agglomerative12(QObject *parent) : QObject(parent) {}
Agglomerative12::~Agglomerative12() = default;

/* ---- Configuration ---- */

void Agglomerative12::setNumClusters(int k) { m_targetK = qMax(1, k); }
void Agglomerative12::setLinkageCentroid() { /* centroid is default */ }

/* ---- Build pairwise distance matrix ---- */

void Agglomerative12::buildDistanceMatrix(const QVector<QVector<double>>& data)
{
    int n = data.size();
    m_distanceMatrix.resize(n);
    for (int i = 0; i < n; ++i) {
        m_distanceMatrix[i].resize(n, 0.0);
        for (int j = i + 1; j < n; ++j) {
            double sum = 0.0;
            int d = qMin(data[i].size(), data[j].size());
            for (int k = 0; k < d; ++k) {
                double diff = data[i][k] - data[j][k];
                sum += diff * diff;
            }
            double dist = qSqrt(sum);
            m_distanceMatrix[i][j] = dist;
            m_distanceMatrix[j][i] = dist;
        }
    }
}

/* ---- Compute centroid of a point set ---- */

QVector<double> Agglomerative12::computeCentroid(
    const QVector<QVector<double>>& data, const QVector<int>& indices) const
{
    if (indices.isEmpty()) return {};
    int d = data[indices[0]].size();
    QVector<double> cen(d, 0.0);
    for (int idx : indices) {
        for (int j = 0; j < d; ++j)
            cen[j] += data[idx][j];
    }
    double inv = 1.0 / indices.size();
    for (int j = 0; j < d; ++j)
        cen[j] *= inv;
    return cen;
}

/* ---- Main agglomerative clustering ---- */

QVector<int> Agglomerative12::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return {};

    m_merges.clear();
    m_labels.resize(n);
    buildDistanceMatrix(data);

    // Track cluster membership: cluster_id -> list of original indices
    QVector<QVector<int>> clusters(n);
    QVector<bool> alive(n, true);
    QVector<QVector<double>> centroids(n);
    for (int i = 0; i < n; ++i) {
        clusters[i].append(i);
        centroids[i] = data[i];
    }

    // Working distance matrix (updated after each merge)
    QVector<QVector<double>> dist = m_distanceMatrix;
    int activeClusters = n;
    int nextId = n; // New cluster IDs

    while (activeClusters > m_targetK) {
        // Find closest pair
        double minDist = std::numeric_limits<double>::max();
        int mergeA = -1, mergeB = -1;
        for (int i = 0; i < nextId; ++i) {
            if (!alive[i]) continue;
            for (int j = i + 1; j < nextId; ++j) {
                if (!alive[j]) continue;
                if (i < dist.size() && j < dist[i].size() && dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    mergeA = i;
                    mergeB = j;
                }
            }
        }
        if (mergeA < 0) break;

        // Record merge
        MergeStep step;
        step.clusterA = mergeA;
        step.clusterB = mergeB;
        step.mergeDistance = minDist;
        step.newSize = clusters[mergeA].size() + clusters[mergeB].size();
        m_merges.append(step);

        // Create merged cluster
        QVector<int> merged = clusters[mergeA];
        merged.append(clusters[mergeB]);
        clusters.append(merged);
        centroids.append(computeCentroid(data, merged));
        alive.append(true);

        // Kill old clusters
        alive[mergeA] = false;
        alive[mergeB] = false;

        // Expand distance matrix for new cluster
        int newId = nextId;
        dist.resize(nextId + 1);
        for (int i = 0; i <= nextId; ++i)
            dist[i].resize(nextId + 1, std::numeric_limits<double>::max());

        // Compute centroid-to-centroid distances to new cluster
        for (int i = 0; i < newId; ++i) {
            if (!alive[i]) continue;
            double d = 0.0;
            int dim = qMin(centroids[i].size(), centroids[newId].size());
            for (int k = 0; k < dim; ++k) {
                double diff = centroids[i][k] - centroids[newId][k];
                d += diff * diff;
            }
            d = qSqrt(d);
            dist[i][newId] = d;
            dist[newId][i] = d;
        }

        nextId++;
        activeClusters--;
    }

    // Assign labels based on remaining alive clusters
    int label = 0;
    for (int i = 0; i < nextId; ++i) {
        if (!alive[i]) continue;
        for (int idx : clusters[i])
            m_labels[idx] = label;
        label++;
    }

    m_stats.numSamples = n;
    m_stats.numDimensions = (n > 0) ? data[0].size() : 0;
    m_stats.numMerges = m_merges.size();
    m_stats.copheneticCorrelation = copheneticCorrelation();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit clusteringCompleted(m_targetK, m_stats.copheneticCorrelation,
                              timer.elapsed());
    return m_labels;
}

/* ---- Cophenetic distance matrix ---- */

QVector<QVector<double>> Agglomerative12::copheneticMatrix(int n) const
{
    // For each pair (i,j), cophenetic distance = merge distance when they first join
    QVector<int> membership(n);
    for (int i = 0; i < n; ++i) membership[i] = i;

    QVector<QVector<int>> clusterMembers(n);
    for (int i = 0; i < n; ++i) clusterMembers[i].append(i);

    QVector<QVector<double>> coph(n, QVector<double>(n, 0.0));

    for (const auto& step : m_merges) {
        int a = step.clusterA;
        int b = step.clusterB;
        // Find all original points in clusters a and b
        QVector<int> membersA, membersB;
        for (int i = 0; i < n; ++i) {
            if (membership[i] == a) membersA.append(i);
            if (membership[i] == b) membersB.append(i);
        }
        // Set cophenetic distance for all cross-pairs
        for (int ia : membersA) {
            for (int ib : membersB) {
                coph[ia][ib] = step.mergeDistance;
                coph[ib][ia] = step.mergeDistance;
            }
        }
        // Merge b into a
        for (int i = 0; i < n; ++i) {
            if (membership[i] == b) membership[i] = a;
        }
    }
    return coph;
}

/* ---- Pearson correlation ---- */

double Agglomerative12::pearsonCorrelation(
    const QVector<QVector<double>>& a,
    const QVector<QVector<double>>& b) const
{
    int n = a.size();
    double sumA = 0.0, sumB = 0.0, sumAA = 0.0, sumBB = 0.0, sumAB = 0.0;
    int count = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double va = a[i][j];
            double vb = b[i][j];
            sumA += va; sumB += vb;
            sumAA += va * va; sumBB += vb * vb;
            sumAB += va * vb;
            count++;
        }
    }
    if (count < 2) return 0.0;
    double num = sumAB - sumA * sumB / count;
    double den = qSqrt((sumAA - sumA * sumA / count) *
                        (sumBB - sumB * sumB / count));
    return (den < 1e-15) ? 0.0 : num / den;
}

/* ---- Cophenetic correlation coefficient ---- */

double Agglomerative12::copheneticCorrelation() const
{
    int n = m_stats.numSamples;
    if (n < 2 || m_merges.isEmpty()) return 0.0;
    auto coph = copheneticMatrix(n);
    return pearsonCorrelation(m_distanceMatrix, coph);
}

/* ---- Accessors ---- */

QVector<Agglomerative12::MergeStep> Agglomerative12::dendrogram() const
{
    return m_merges;
}

/* ---- Reset ---- */

void Agglomerative12::resetStatistics()
{
    m_merges.clear();
    m_distanceMatrix.clear();
    m_labels.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}

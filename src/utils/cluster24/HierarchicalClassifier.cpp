/**
 * @file HierarchicalClassifier.cpp
 * @brief 层次分类器实现
 */

#include "utils/cluster24/HierarchicalClassifier.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

HierarchicalClassifier::HierarchicalClassifier(QObject* parent)
    : QObject(parent)
    , m_targetClusters(2)
    , m_strategy(Strategy::Agglomerative)
    , m_linkage(Linkage::Average)
    , m_distanceThreshold(0.0)
    , m_timeSum(0.0)
{
}

void HierarchicalClassifier::setTargetClusters(int k)
{
    m_targetClusters = qMax(1, k);
}

void HierarchicalClassifier::setStrategy(Strategy strategy)
{
    m_strategy = strategy;
}

void HierarchicalClassifier::setLinkage(Linkage linkage)
{
    m_linkage = linkage;
}

void HierarchicalClassifier::setDistanceThreshold(double threshold)
{
    m_distanceThreshold = qMax(0.0, threshold);
}

QList<HierarchicalClassifier::Cluster> HierarchicalClassifier::classify(
    const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Cluster> result;
    if (data.size() < 2) return result;

    if (m_strategy == Strategy::Agglomerative) {
        result = agglomerativeCluster(data);
    } else {
        result = divisiveCluster(data);
    }

    ++m_stats.totalClusterings;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit classificationComplete(result.size(), timer.elapsed());
    return result;
}

QList<HierarchicalClassifier::MergeStep> HierarchicalClassifier::dendrogram() const
{
    return m_mergeHistory;
}

double HierarchicalClassifier::silhouetteScore(
    const QVector<double>& data, const QList<Cluster>& clusters) const
{
    if (clusters.size() < 2 || data.isEmpty()) return 0.0;

    int n = data.size();
    QVector<int> assignments(n, -1);
    for (int c = 0; c < clusters.size(); ++c) {
        for (int idx : clusters[c].memberIndices) {
            if (idx < n) assignments[idx] = c;
        }
    }

    double totalScore = 0.0;
    int validCount = 0;
    for (int i = 0; i < n; ++i) {
        if (assignments[i] < 0) continue;
        int myCluster = assignments[i];

        /* a: 簇内平均距离 */
        double a = 0.0;
        int aCount = 0;
        for (int idx : clusters[myCluster].memberIndices) {
            if (idx != i) { a += qAbs(data[i] - data[idx]); ++aCount; }
        }
        a = (aCount > 0) ? a / aCount : 0.0;

        /* b: 最近其他簇平均距离 */
        double b = std::numeric_limits<double>::max();
        for (int c = 0; c < clusters.size(); ++c) {
            if (c == myCluster) continue;
            double avgDist = 0.0;
            for (int idx : clusters[c].memberIndices) avgDist += qAbs(data[i] - data[idx]);
            if (!clusters[c].memberIndices.isEmpty())
                avgDist /= clusters[c].memberIndices.size();
            if (avgDist < b) b = avgDist;
        }
        totalScore += (b - a) / qMax(a, b);
        ++validCount;
    }
    return (validCount > 0) ? totalScore / validCount : 0.0;
}

QList<HierarchicalClassifier::Cluster> HierarchicalClassifier::agglomerativeCluster(
    const QVector<double>& data)
{
    int n = data.size();
    /* 初始化: 每个点自成一簇 */
    QVector<QVector<int>> clusters(n);
    QVector<double> centroids(n);
    for (int i = 0; i < n; ++i) {
        clusters[i].append(i);
        centroids[i] = data[i];
    }
    m_mergeHistory.clear();

    /* 距离矩阵 */
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dist[i][j] = dist[j][i] = qAbs(data[i] - data[j]);
        }
    }

    QVector<bool> alive(n, true);
    int aliveCount = n;

    while (aliveCount > m_targetClusters) {
        /* 找最小距离对 */
        double minDist = std::numeric_limits<double>::max();
        int bestA = -1, bestB = -1;
        for (int i = 0; i < n; ++i) {
            if (!alive[i]) continue;
            for (int j = i + 1; j < n; ++j) {
                if (!alive[j]) continue;
                if (dist[i][j] < minDist) {
                    minDist = dist[i][j];
                    bestA = i;
                    bestB = j;
                }
            }
        }
        if (bestA < 0) break;

        /* 距离阈值截断 */
        if (m_distanceThreshold > 0.0 && minDist > m_distanceThreshold) break;

        /* 合并 bestA ← bestB */
        MergeStep step;
        step.clusterA = bestA;
        step.clusterB = bestB;
        step.distance = minDist;
        step.newSize = clusters[bestA].size() + clusters[bestB].size();
        m_mergeHistory.append(step);
        ++m_stats.totalMerges;

        clusters[bestA].append(clusters[bestB]);
        clusters[bestB].clear();
        alive[bestB] = false;

        /* 更新距离矩阵 */
        for (int k = 0; k < n; ++k) {
            if (!alive[k] || k == bestA) continue;
            double newDist = clusterDistance(data, clusters[bestA], clusters[k]);
            dist[bestA][k] = dist[k][bestA] = newDist;
        }
        --aliveCount;

        emit mergePerformed(aliveCount + 1, aliveCount, minDist);
    }

    /* 构造输出 */
    QList<Cluster> result;
    for (int i = 0; i < n; ++i) {
        if (!alive[i] || clusters[i].isEmpty()) continue;
        Cluster cl;
        cl.memberIndices = clusters[i];
        double sum = 0.0;
        for (int idx : cl.memberIndices) sum += data[idx];
        cl.centroid = sum / cl.memberIndices.size();
        double varSum = 0.0;
        for (int idx : cl.memberIndices) {
            double d = data[idx] - cl.centroid;
            varSum += d * d;
        }
        cl.variance = (cl.memberIndices.size() > 1)
                          ? varSum / cl.memberIndices.size() : 0.0;
        result.append(cl);
    }
    return result;
}

QList<HierarchicalClassifier::Cluster> HierarchicalClassifier::divisiveCluster(
    const QVector<double>& data)
{
    int n = data.size();
    /* 初始: 所有点在一个簇 */
    QList<QVector<int>> clusters;
    QVector<int> allIndices;
    for (int i = 0; i < n; ++i) allIndices.append(i);
    clusters.append(allIndices);
    m_mergeHistory.clear();

    while (clusters.size() < m_targetClusters) {
        /* 找方差最大的簇进行分裂 */
        int splitIdx = -1;
        double maxVar = -1.0;
        for (int c = 0; c < clusters.size(); ++c) {
            if (clusters[c].size() < 2) continue;
            double mean = 0.0;
            for (int idx : clusters[c]) mean += data[idx];
            mean /= clusters[c].size();
            double var = 0.0;
            for (int idx : clusters[c]) {
                double d = data[idx] - mean;
                var += d * d;
            }
            if (var > maxVar) { maxVar = var; splitIdx = c; }
        }
        if (splitIdx < 0) break;

        /* 用K-Means(K=2)分裂 */
        const QVector<int>& target = clusters[splitIdx];
        double mean = 0.0;
        for (int idx : target) mean += data[idx];
        mean /= target.size();

        double cenA = data[target.first()];
        double cenB = data[target.last()];
        QVector<int> groupA, groupB;
        for (int iter = 0; iter < 20; ++iter) {
            groupA.clear();
            groupB.clear();
            for (int idx : target) {
                if (qAbs(data[idx] - cenA) <= qAbs(data[idx] - cenB))
                    groupA.append(idx);
                else
                    groupB.append(idx);
            }
            if (groupA.isEmpty() || groupB.isEmpty()) break;
            double sumA = 0.0, sumB = 0.0;
            for (int idx : groupA) sumA += data[idx];
            for (int idx : groupB) sumB += data[idx];
            cenA = sumA / groupA.size();
            cenB = sumB / groupB.size();
        }
        if (groupA.isEmpty() || groupB.isEmpty()) break;

        MergeStep step;
        step.clusterA = splitIdx;
        step.clusterB = clusters.size();
        step.distance = qAbs(cenA - cenB);
        step.newSize = 0;
        m_mergeHistory.append(step);
        ++m_stats.totalMerges;

        clusters[splitIdx] = groupA;
        clusters.append(groupB);
        emit mergePerformed(clusters.size() - 1, clusters.size(), step.distance);
    }

    /* 构造输出 */
    QList<Cluster> result;
    for (const auto& members : clusters) {
        if (members.isEmpty()) continue;
        Cluster cl;
        cl.memberIndices = members;
        double sum = 0.0;
        for (int idx : cl.memberIndices) sum += data[idx];
        cl.centroid = sum / cl.memberIndices.size();
        double varSum = 0.0;
        for (int idx : cl.memberIndices) {
            double d = data[idx] - cl.centroid;
            varSum += d * d;
        }
        cl.variance = (cl.memberIndices.size() > 1)
                          ? varSum / cl.memberIndices.size() : 0.0;
        result.append(cl);
    }
    return result;
}

double HierarchicalClassifier::clusterDistance(const QVector<double>& data,
    const QVector<int>& a, const QVector<int>& b) const
{
    if (a.isEmpty() || b.isEmpty()) return 0.0;

    switch (m_linkage) {
    case Linkage::Single: {
        double minD = std::numeric_limits<double>::max();
        for (int i : a)
            for (int j : b)
                minD = qMin(minD, qAbs(data[i] - data[j]));
        return minD;
    }
    case Linkage::Complete: {
        double maxD = 0.0;
        for (int i : a)
            for (int j : b)
                maxD = qMax(maxD, qAbs(data[i] - data[j]));
        return maxD;
    }
    case Linkage::Average: {
        double totalDist = 0.0;
        int count = 0;
        for (int i : a) {
            for (int j : b) {
                totalDist += qAbs(data[i] - data[j]);
                ++count;
            }
        }
        return (count > 0) ? totalDist / count : 0.0;
    }
    case Linkage::Ward:
        return wardDistance(data, a, b);
    }
    return 0.0;
}

double HierarchicalClassifier::wardDistance(const QVector<double>& data,
    const QVector<int>& a, const QVector<int>& b) const
{
    if (a.isEmpty() || b.isEmpty()) return 0.0;
    double meanA = 0.0, meanB = 0.0;
    for (int i : a) meanA += data[i];
    for (int j : b) meanB += data[j];
    meanA /= a.size();
    meanB /= b.size();
    double d = meanA - meanB;
    return (2.0 * a.size() * b.size()) / (a.size() + b.size()) * d * d;
}

void HierarchicalClassifier::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_mergeHistory.clear();
}

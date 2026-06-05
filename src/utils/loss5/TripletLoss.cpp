/**
 * @file TripletLoss.cpp
 * @brief 三元组损失函数实现,用于度量学习和嵌入训练
 */

#include "TripletLoss.h"
#include <QElapsedTimer>
#include <QMap>
#include <QVector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>

TripletLoss::TripletLoss(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

double TripletLoss::computeLoss(const QVector<double>& anchor,
                                const QVector<double>& positive,
                                const QVector<double>& negative,
                                double margin,
                                DistanceMetric metric)
{
    double dAp = distance(anchor, positive, metric);
    double dAn = distance(anchor, negative, metric);
    return std::max(0.0, dAp - dAn + margin);
}

QPair<double, QVector<TripletLoss::Triplet>> TripletLoss::batchLoss(
    const QVector<QVector<double>>& embeddings,
    const QVector<int>& labels,
    double margin,
    MiningStrategy strategy,
    DistanceMetric metric)
{
    QElapsedTimer timer;
    timer.start();

    int n = embeddings.size();
    QVector<Triplet> triplets;
    double totalLoss = 0.0;

    if (n == 0 || labels.size() != n) {
        m_timeSum += timer.elapsed();
        return {0.0, triplets};
    }

    /* 预计算所有点对的距离矩阵 */
    QVector<QVector<double>> distMat(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            double d = distance(embeddings[i], embeddings[j], metric);
            distMat[i][j] = d;
            distMat[j][i] = d;
        }

    /* 按标签分组 */
    QMap<int, QVector<int>> labelGroups;
    for (int i = 0; i < n; ++i)
        labelGroups[labels[i]].append(i);

    std::mt19937 rng(42);

    for (int a = 0; a < n; ++a) {
        /* 找正例: 与anchor同标签的其他点 */
        const QVector<int>& sameLabel = labelGroups[labels[a]];
        if (sameLabel.size() <= 1) continue;

        QVector<int> positives;
        for (int p : sameLabel)
            if (p != a) positives.append(p);

        for (int pIdx : positives) {
            double dAp = distMat[a][pIdx];
            int negIdx = -1;

            if (strategy == All) {
                /* 使用所有负例 */
                for (int neg = 0; neg < n; ++neg) {
                    if (labels[neg] == labels[a]) continue;
                    double dAn = distMat[a][neg];
                    double loss = std::max(0.0, dAp - dAn + margin);
                    if (loss > 0.0) {
                        Triplet t;
                        t.anchor = embeddings[a];
                        t.positive = embeddings[pIdx];
                        t.negative = embeddings[neg];
                        t.anchorPosDist = dAp;
                        t.anchorNegDist = dAn;
                        t.loss = loss;
                        triplets.append(t);
                        totalLoss += loss;
                    }
                }
            } else if (strategy == Hard) {
                /* 硬负例: 距anchor最近的异类样本 */
                double minNegDist = 1e30;
                for (int neg = 0; neg < n; ++neg) {
                    if (labels[neg] == labels[a]) continue;
                    if (distMat[a][neg] < minNegDist) {
                        minNegDist = distMat[a][neg];
                        negIdx = neg;
                    }
                }
            } else {
                /* 半硬负例: dAp < d_an < dAp + margin */
                negIdx = mineSemiHardNegative(a, embeddings, labels,
                                              dAp, metric);
                if (negIdx < 0) {
                    /* 找不到半硬负例,退化为硬负例 */
                    double minNegDist = 1e30;
                    for (int neg = 0; neg < n; ++neg) {
                        if (labels[neg] == labels[a]) continue;
                        if (distMat[a][neg] < minNegDist) {
                            minNegDist = distMat[a][neg];
                            negIdx = neg;
                        }
                    }
                }
            }

            /* 对于Hard和SemiHard策略,记录选中的三元组 */
            if ((strategy == Hard || strategy == SemiHard) && negIdx >= 0) {
                double dAn = distMat[a][negIdx];
                double loss = std::max(0.0, dAp - dAn + margin);
                Triplet t;
                t.anchor = embeddings[a];
                t.positive = embeddings[pIdx];
                t.negative = embeddings[negIdx];
                t.anchorPosDist = dAp;
                t.anchorNegDist = dAn;
                t.loss = loss;
                triplets.append(t);
                totalLoss += loss;
            }
        }
    }

    double meanLoss = triplets.isEmpty() ? 0.0 : totalLoss / triplets.size();

    m_stats.totalLossComputed++;
    m_stats.totalTripletsMined += triplets.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalLossComputed;

    emit batchLossComputed(meanLoss, triplets.size());
    return {meanLoss, triplets};
}

int TripletLoss::mineSemiHardNegative(
    int anchorIdx,
    const QVector<QVector<double>>& embeddings,
    const QVector<int>& labels,
    double anchorPosDist,
    DistanceMetric metric)
{
    int n = embeddings.size();
    int anchorLabel = labels[anchorIdx];

    QVector<int> candidates;

    /* 收集所有满足 dAp < d_an 的负例 */
    for (int i = 0; i < n; ++i) {
        if (labels[i] == anchorLabel) continue;
        double dAn = distance(embeddings[anchorIdx], embeddings[i], metric);
        if (dAn > anchorPosDist)
            candidates.append(i);
    }

    if (candidates.isEmpty()) return -1;

    /* 在候选中选择距离最小的(半硬) */
    double bestDist = 1e30;
    int bestIdx = -1;
    for (int c : candidates) {
        double d = distance(embeddings[anchorIdx], embeddings[c], metric);
        if (d < bestDist) {
            bestDist = d;
            bestIdx = c;
        }
    }

    return bestIdx;
}

double TripletLoss::distance(const QVector<double>& a,
                             const QVector<double>& b,
                             DistanceMetric metric) const
{
    switch (metric) {
    case Euclidean:  return euclideanDist(a, b);
    case Cosine:     return cosineDist(a, b);
    case Manhattan:  return manhattanDist(a, b);
    }
    return euclideanDist(a, b);
}

double TripletLoss::euclideanDist(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return std::sqrt(sum);
}

double TripletLoss::cosineDist(const QVector<double>& a,
                               const QVector<double>& b) const
{
    int dim = qMin(a.size(), b.size());
    double dot = 0.0, normA = 0.0, normB = 0.0;
    for (int i = 0; i < dim; ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    double denom = std::sqrt(normA) * std::sqrt(normB);
    if (denom < 1e-15) return 1.0;
    double cosSim = dot / denom;
    return 1.0 - cosSim;
}

double TripletLoss::manhattanDist(const QVector<double>& a,
                                  const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i)
        sum += std::abs(a[i] - b[i]);
    return sum;
}

TripletLoss::Stats TripletLoss::stats() const { return m_stats; }

void TripletLoss::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

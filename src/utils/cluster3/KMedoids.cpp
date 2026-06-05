/**
 * @file KMedoids.cpp
 * @brief K-Medoids聚类引擎实现 — PAM算法
 */

#include "KMedoids.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <limits>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

KMedoids::KMedoids(QObject* parent)
    : QObject(parent)
{
}

KMedoids::~KMedoids() = default;

// ═══════════════════════════════════════════════════════════
// 拟合
// ═══════════════════════════════════════════════════════════

KMedoids::FitResult KMedoids::fit(const QVector<Point>& data, int k,
                                   int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;

    if (data.isEmpty() || k <= 0 || k > data.size()) {
        m_stats.totalFits++;
        emit fitCompleted(k, 0);
        return result;
    }

    const int n = data.size();

    /* BUILD阶段: 随机选择k个初始Medoid */
    QVector<int> medoidIndices;
    QVector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(),
                 *QRandomGenerator::global());

    for (int i = 0; i < k; ++i) {
        medoidIndices.append(indices[i]);
    }

    /* 初始分配 */
    QVector<int> labels(n, -1);
    double totalCost = computeCost(data, medoidIndices, labels);

    /* SWAP阶段: 迭代优化 */
    int iter = 0;
    bool changed = true;

    while (changed && iter < maxIter) {
        changed = false;

        for (int mi = 0; mi < k; ++mi) {
            int currentMedoid = medoidIndices[mi];
            double bestCost = totalCost;
            int bestCandidate = currentMedoid;

            /* 尝试与非Medoid点交换 */
            for (int candidate = 0; candidate < n; ++candidate) {
                if (candidate == currentMedoid) continue;

                /* 检查candidate是否已是Medoid */
                bool isMedoid = false;
                for (int mj = 0; mj < k; ++mj) {
                    if (medoidIndices[mj] == candidate) {
                        isMedoid = true;
                        break;
                    }
                }
                if (isMedoid) continue;

                /* 试交换 */
                QVector<int> trialIndices = medoidIndices;
                trialIndices[mi] = candidate;
                QVector<int> trialLabels(n, -1);
                double trialCost = computeCost(data, trialIndices, trialLabels);

                if (trialCost < bestCost) {
                    bestCost = trialCost;
                    bestCandidate = candidate;
                }
            }

            if (bestCandidate != currentMedoid) {
                medoidIndices[mi] = bestCandidate;
                totalCost = bestCost;
                changed = true;
            }
        }

        ++iter;
    }

    /* 最终分配 */
    totalCost = computeCost(data, medoidIndices, labels);

    result.labels = labels;
    result.medoidIndices = medoidIndices;
    result.iterations = iter;
    result.totalCost = totalCost;
    result.converged = !changed || iter >= maxIter;

    /* 保存Medoid点用于predict */
    m_medoids.clear();
    for (int idx : medoidIndices) {
        m_medoids.append(data[idx]);
    }
    m_fitted = true;

    /* 更新统计 */
    m_stats.totalFits++;
    const qint64 elapsed = timer.elapsed();
    const auto total = m_stats.totalFits + m_stats.totalPredictions;
    if (total == 1) {
        m_stats.avgProcessingTimeMs = static_cast<double>(elapsed);
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs * (total - 1) / total +
            static_cast<double>(elapsed) / total;
    }

    emit fitCompleted(k, iter);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 预测
// ═══════════════════════════════════════════════════════════

int KMedoids::predict(const Point& point) const
{
    if (!m_fitted || m_medoids.isEmpty()) {
        return -1;
    }

    int bestCluster = 0;
    double bestDist = std::numeric_limits<double>::max();

    for (int i = 0; i < m_medoids.size(); ++i) {
        double d = distance(point, m_medoids[i]);
        if (d < bestDist) {
            bestDist = d;
            bestCluster = i;
        }
    }

    return bestCluster;
}

bool KMedoids::isFitted() const
{
    return m_fitted;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

KMedoids::Stats KMedoids::stats() const
{
    return m_stats;
}

void KMedoids::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部实现
// ═══════════════════════════════════════════════════════════

double KMedoids::distance(const Point& a, const Point& b) const
{
    const int dims = std::min(a.size(), b.size());
    double sum = 0.0;
    for (int i = 0; i < dims; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

double KMedoids::computeCost(const QVector<Point>& data,
                              const QVector<int>& medoidIndices,
                              QVector<int>& labels) const
{
    const int n = data.size();
    const int k = medoidIndices.size();
    double totalCost = 0.0;

    for (int i = 0; i < n; ++i) {
        double bestDist = std::numeric_limits<double>::max();
        int bestCluster = 0;

        for (int c = 0; c < k; ++c) {
            double d = distance(data[i], data[medoidIndices[c]]);
            if (d < bestDist) {
                bestDist = d;
                bestCluster = c;
            }
        }

        labels[i] = bestCluster;
        totalCost += bestDist;
    }

    return totalCost;
}

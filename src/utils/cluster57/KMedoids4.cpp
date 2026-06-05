/**
 * @file KMedoids4.cpp
 * @brief K-Medoids聚类算法实现 (PAM算法)
 *
 * 实现基于PAM (Partitioning Around Medoids) 的K-Medoids聚类算法。
 * 支持欧氏距离、曼哈顿距离和余弦距离三种距离度量方式。
 * 通过迭代交换medoid来最小化总代价函数，直到收敛或达到最大迭代次数。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/cluster57/KMedoids4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化K-Medoids聚类器
 * @param parent 父QObject指针
 */
KMedoids4::KMedoids4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类数量
 * @param k 目标聚类数量，必须大于0
 */
void KMedoids4::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数，必须大于0
 */
void KMedoids4::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置距离度量方式
 * @param metric 距离度量名称，支持 "euclidean"(欧氏)、"manhattan"(曼哈顿)、"cosine"(余弦)
 */
void KMedoids4::setDistanceMetric(const QString& metric)
{
    if (metric == "euclidean" || metric == "manhattan" || metric == "cosine") {
        m_metric = metric;
    }
}

/**
 * @brief 执行K-Medoids聚类
 *
 * 使用PAM算法进行聚类:
 * 1. 随机选择k个初始medoid
 * 2. 将每个点分配到最近的medoid
 * 3. 对每个medoid，尝试与非medoid点交换
 * 4. 如果交换后总代价减小，则接受交换
 * 5. 重复直到收敛或达到最大迭代次数
 *
 * @param points 输入数据点集合，每个点是一个特征向量
 * @return 每个数据点的聚类标签 (0 ~ k-1)
 */
QVector<int> KMedoids4::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    QVector<int> labels(n, 0);

    /* 数据不足时直接返回全0标签 */
    if (n == 0 || m_k <= 0) {
        m_medoids.clear();
        m_totalCost = 0.0;
        emit clusteringCompleted(0, 0.0);
        return labels;
    }

    /* 若数据点数小于k，则每个点自身即为medoid */
    int effectiveK = qMin(m_k, n);

    /* 使用随机数生成器选取初始medoid */
    std::mt19937 rng(static_cast<unsigned>(qrand()));
    QVector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);

    m_medoids = QVector<int>(indices.begin(), indices.begin() + effectiveK);

    /* PAM算法迭代 */
    double prevCost = assignCost(points, m_medoids);
    bool changed = true;
    int iter = 0;

    while (changed && iter < m_maxIter) {
        changed = false;
        iter++;

        /* 对每个medoid尝试与非medoid点交换 */
        for (int i = 0; i < effectiveK; ++i) {
            double bestSwapCost = prevCost;
            int bestSwapCandidate = -1;

            for (int j = 0; j < n; ++j) {
                /* 跳过已经是medoid的点 */
                if (m_medoids.contains(j)) {
                    continue;
                }

                /* 尝试交换: 用点j替换第i个medoid */
                QVector<int> trialMedoids = m_medoids;
                trialMedoids[i] = j;
                double trialCost = assignCost(points, trialMedoids);

                if (trialCost < bestSwapCost) {
                    bestSwapCost = trialCost;
                    bestSwapCandidate = j;
                }
            }

            /* 如果找到更好的medoid，执行交换 */
            if (bestSwapCandidate >= 0 && bestSwapCost < prevCost) {
                m_medoids[i] = bestSwapCandidate;
                prevCost = bestSwapCost;
                changed = true;
            }
        }
    }

    m_totalCost = prevCost;

    /* 根据最终medoid分配每个点的标签 */
    for (int i = 0; i < n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        for (int j = 0; j < effectiveK; ++j) {
            double d = distance(points[i], points[m_medoids[j]]);
            if (d < minDist) {
                minDist = d;
                labels[i] = j;
            }
        }
    }

    /* 更新统计信息 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(effectiveK, m_totalCost);
    return labels;
}

/**
 * @brief 重置所有统计数据
 */
void KMedoids4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算两个数据点之间的距离
 * @param a 第一个数据点
 * @param b 第二个数据点
 * @return 两点之间的距离值
 *
 * 根据当前设置的度量方式计算距离:
 * - euclidean: 欧氏距离 sqrt(sum((ai-bi)^2))
 * - manhattan: 曼哈顿距离 sum(|ai-bi|)
 * - cosine: 余弦距离 1 - cos(a,b)
 */
double KMedoids4::distance(const QVector<double>& a, const QVector<double>& b) const
{
    const int dim = qMin(a.size(), b.size());
    if (dim == 0) return 0.0;

    if (m_metric == "manhattan") {
        /* 曼哈顿距离 */
        double sum = 0.0;
        for (int i = 0; i < dim; ++i) {
            sum += qAbs(a[i] - b[i]);
        }
        return sum;
    }

    if (m_metric == "cosine") {
        /* 余弦距离: 1 - cos_similarity */
        double dotProd = 0.0;
        double normA = 0.0;
        double normB = 0.0;
        for (int i = 0; i < dim; ++i) {
            dotProd += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        double denom = qSqrt(normA) * qSqrt(normB);
        if (denom < 1e-12) return 1.0;
        return 1.0 - dotProd / denom;
    }

    /* 默认: 欧氏距离 */
    double sum = 0.0;
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 计算给定medoid集合的总分配代价
 *
 * 总代价 = 所有点到其最近medoid的距离之和
 *
 * @param pts 数据点集合
 * @param meds 当前medoid索引列表
 * @return 总分配代价
 */
double KMedoids4::assignCost(const QVector<QVector<double>>& pts, const QVector<int>& meds)
{
    double total = 0.0;
    const int n = pts.size();
    const int k = meds.size();

    for (int i = 0; i < n; ++i) {
        double minDist = std::numeric_limits<double>::max();
        for (int j = 0; j < k; ++j) {
            double d = distance(pts[i], pts[meds[j]]);
            if (d < minDist) {
                minDist = d;
            }
        }
        total += minDist;
    }

    return total;
}

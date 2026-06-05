#include "KMeans10.h"
#include <QElapsedTimer>
#include <cmath>
#include <random>
#include <algorithm>

/**
 * @brief 构造函数，初始化K-Means聚类器
 * @param parent 父对象指针
 */
KMeans10::KMeans10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置聚类簇数量
 * @param count 目标簇数量，必须大于0
 */
void KMeans10::setClusterCount(int count)
{
    m_clusterCount = qMax(1, count);
}

/**
 * @brief 对输入数据集执行K-Means拟合
 *
 * 使用K-Means++初始化策略选取初始聚类中心，
 * 然后迭代执行分配和更新步骤直到收敛或达到最大迭代次数。
 *
 * @param data 输入数据集，每个元素为一个样本的特征向量
 */
void KMeans10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty() || m_clusterCount <= 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalClusterings++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
        emit clusteringCompleted(0, 0);
        return;
    }

    const int n = data.size();
    const int dim = data[0].size();
    const int k = qMin(m_clusterCount, n);
    const int maxIter = 300;

    /* K-Means++ 初始化：按距离概率选取初始中心 */
    m_centroids.clear();
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uni(0, n - 1);
    m_centroids.push_back(data[uni(rng)]);

    for (int c = 1; c < k; ++c) {
        QVector<double> dists(n, 0.0);
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double minD = 1e18;
            for (const auto& cent : m_centroids) {
                double d = 0.0;
                for (int j = 0; j < dim; ++j) {
                    double diff = data[i][j] - cent[j];
                    d += diff * diff;
                }
                minD = qMin(minD, d);
            }
            dists[i] = minD;
            totalDist += minD;
        }
        /* 轮盘赌选择下一个中心 */
        std::uniform_real_distribution<double> prob(0.0, totalDist);
        double threshold = prob(rng);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cumSum += dists[i];
            if (cumSum >= threshold) { chosen = i; break; }
        }
        m_centroids.push_back(data[chosen]);
    }

    /* 迭代优化：分配 + 更新中心 */
    QVector<int> labels(n, 0);
    int iterations = 0;
    for (int iter = 0; iter < maxIter; ++iter) {
        iterations = iter + 1;
        bool changed = false;

        /* 分配每个样本到最近的中心 */
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestLabel = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int j = 0; j < dim; ++j) {
                    double diff = data[i][j] - m_centroids[c][j];
                    d += diff * diff;
                }
                if (d < bestDist) { bestDist = d; bestLabel = c; }
            }
            if (labels[i] != bestLabel) { labels[i] = bestLabel; changed = true; }
        }

        if (!changed) break;

        /* 重新计算每个簇的中心 */
        for (int c = 0; c < k; ++c) {
            QVector<double> sum(dim, 0.0);
            int count = 0;
            for (int i = 0; i < n; ++i) {
                if (labels[i] == c) {
                    for (int j = 0; j < dim; ++j) sum[j] += data[i][j];
                    count++;
                }
            }
            if (count > 0) {
                for (int j = 0; j < dim; ++j) m_centroids[c][j] = sum[j] / count;
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    emit clusteringCompleted(k, iterations);
}

/**
 * @brief 预测单个样本所属簇编号
 *
 * 计算样本到每个聚类中心的欧氏距离，返回最近中心的编号。
 *
 * @param sample 待预测样本的特征向量
 * @return 最近的簇编号，若未拟合则返回0
 */
int KMeans10::predict(const QVector<double>& sample)
{
    QElapsedTimer timer;
    timer.start();

    if (m_centroids.isEmpty() || sample.isEmpty()) {
        return 0;
    }

    int dim = qMin(sample.size(), m_centroids[0].size());
    double bestDist = 1e18;
    int bestLabel = 0;

    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = 0.0;
        for (int j = 0; j < dim; ++j) {
            double diff = sample[j] - m_centroids[c][j];
            d += diff * diff;
        }
        if (d < bestDist) { bestDist = d; bestLabel = c; }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;
    return bestLabel;
}

/**
 * @brief 获取当前聚类中心坐标
 * @return 聚类中心向量，每个元素为一个中心的特征坐标
 */
QVector<QVector<double>> KMeans10::centroids() const
{
    return m_centroids;
}

/**
 * @brief 重置统计数据
 */
void KMeans10::resetStatistics()
{
    m_stats.totalClusterings = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

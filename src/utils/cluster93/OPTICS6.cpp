#include "OPTICS6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化OPTICS聚类器
 * @param parent 父对象指针
 */
OPTICS6::OPTICS6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置邻域半径上限
 * @param epsilon 最大邻域半径
 */
void OPTICS6::setEpsilon(double epsilon)
{
    m_epsilon = qMax(0.001, epsilon);
}

/**
 * @brief 设置最小核心点数
 * @param minPts 成为核心点所需的最小邻居数
 */
void OPTICS6::setMinPts(int minPts)
{
    m_minPts = qMax(1, minPts);
}

/**
 * @brief 计算两点间的欧氏距离
 * @param a 第一个点
 * @param b 第二个点
 * @return 欧氏距离
 */
static double eucDist(const QVector<double>& a, const QVector<double>& b)
{
    double d = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        d += diff * diff;
    }
    return std::sqrt(d);
}

/**
 * @brief 计算核心距离
 * @param pointIdx 中心点索引
 * @param neighbors 邻居索引列表
 * @param data 数据集
 * @param minPts 最小点数
 * @return 核心距离(第minPts近邻的距离)，若非核心点返回无穷大
 */
static double coreDistance(int pointIdx, const QVector<int>& neighbors,
                            const QVector<QVector<double>>& data, int minPts)
{
    if (neighbors.size() < minPts) return 1e18;
    QVector<double> dists;
    for (int n : neighbors) {
        dists.append(eucDist(data[pointIdx], data[n]));
    }
    std::sort(dists.begin(), dists.end());
    return dists[qMin(minPts - 1, dists.size() - 1)];
}

/**
 * @brief 对输入数据执行OPTICS聚类分析
 *
 * 生成可达距离排序序列：
 * 1. 从未处理点中选择种子
 * 2. 计算核心距离和可达距离
 * 3. 按可达距离排序更新种子列表
 * 4. 输出有序的点序列
 *
 * @param data 输入数据集
 */
void OPTICS6::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalClustered++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
        emit clusteringCompleted(0);
        return;
    }

    const int n = data.size();
    QVector<bool> processed(n, false);
    QVector<double> reachability(n, 1e18); /* 初始可达距离为无穷大 */

    /* 预计算所有邻域 */
    QVector<QVector<int>> allNeighbors(n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (eucDist(data[i], data[j]) <= m_epsilon) {
                allNeighbors[i].append(j);
            }
        }
    }

    int processedCount = 0;

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        /* 处理当前点 */
        processed[i] = true;
        processedCount++;

        double cDist = coreDistance(i, allNeighbors[i], data, m_minPts);

        if (cDist < 1e17) {
            /* 核心点：更新邻居的可达距离 */
            QVector<QPair<double, int>> seeds; /* (reachDist, index) */
            for (int nb : allNeighbors[i]) {
                if (processed[nb]) continue;
                double newReach = qMax(cDist, eucDist(data[i], data[nb]));
                if (newReach < reachability[nb]) {
                    reachability[nb] = newReach;
                }
                seeds.append({reachability[nb], nb});
            }

            /* 按可达距离排序处理种子 */
            std::sort(seeds.begin(), seeds.end());
            for (auto& seed : seeds) {
                int idx = seed.second;
                if (processed[idx]) continue;
                processed[idx] = true;
                processedCount++;

                double seedCDist = coreDistance(idx, allNeighbors[idx], data, m_minPts);
                if (seedCDist < 1e17) {
                    for (int nb : allNeighbors[idx]) {
                        if (processed[nb]) continue;
                        double newReach = qMax(seedCDist, eucDist(data[idx], data[nb]));
                        if (newReach < reachability[nb]) {
                            reachability[nb] = newReach;
                        }
                    }
                }
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalClustered++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;
    emit clusteringCompleted(processedCount);
}

/**
 * @brief 重置统计数据
 */
void OPTICS6::resetStatistics()
{
    m_stats.totalClustered = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}

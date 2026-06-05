#include "BirchClustering9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file BirchClustering9.cpp
 * @brief BIRCH聚类算法实现
 *
 * BIRCH(Balanced Iterative Reducing and Clustering using Hierarchies)
 * 利用聚类特征(CF)向量和CF树进行增量式聚类，适用于大规模数据。
 * CF = (n, LS, SS) 其中n为点数，LS为线性和，SS为平方和。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
BirchClustering9::BirchClustering9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CF树分支因子
 * @param factor 每个内部节点的最大子节点数
 */
void BirchClustering9::setBranchFactor(int factor)
{
    m_branchFactor = qMax(2, factor);
}

/**
 * @brief 设置聚类半径阈值
 * @param threshold 子簇的最大半径，超过则分裂
 */
void BirchClustering9::setThreshold(double threshold)
{
    m_threshold = qMax(0.001, threshold);
}

/**
 * @brief 计算两个CF之间的距离
 * @param ls1 第一个CF的线性和
 * @param n1 第一个CF的点数
 * @param ls2 第二个CF的线性和
 * @param n2 第二个CF的点数
 * @return 两个CF质心之间的欧氏距离
 */
static double cfDistance(const QVector<double>& ls1, int n1,
                         const QVector<double>& ls2, int n2)
{
    const int dims = ls1.size();
    double dist = 0.0;
    for (int d = 0; d < dims; ++d) {
        const double diff = ls1[d] / n1 - ls2[d] / n2;
        dist += diff * diff;
    }
    return std::sqrt(dist);
}

/**
 * @brief 对输入数据执行BIRCH聚类
 *
 * BIRCH算法流程:
 * 1. 逐点插入CF树: 找到最近的叶条目，合并或分裂
 * 2. 可选: 对叶条目进行全局聚类
 *
 * @param data 输入数据矩阵
 */
void BirchClustering9::fit(const QVector<QVector<double>>& data)
{
    if (data.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    const int dims = data[0].size();

    // CF条目: (点数, 线性和向量)
    QVector<int> cfCount;                ///< 每个CF的点数
    QVector<QVector<double>> cfLS;       ///< 每个CF的线性和
    QVector<QVector<double>> cfCentroid; ///< 每个CF的质心

    // 逐点构建CF树(简化: 直接维护叶级CF列表)
    for (int i = 0; i < n; ++i) {
        bool merged = false;

        // 寻找最近的CF条目
        double minDist = 1e18;
        int bestCF = -1;

        for (int j = 0; j < cfCount.size(); ++j) {
            const double dist = cfDistance(cfLS[j], cfCount[j],
                                            data[i], 1);
            if (dist < minDist) {
                minDist = dist;
                bestCF = j;
            }
        }

        // 如果距离在阈值内，合并到该CF
        if (bestCF >= 0 && minDist < m_threshold) {
            cfCount[bestCF]++;
            for (int d = 0; d < dims; ++d) {
                cfLS[bestCF][d] += data[i][d];
            }
            merged = true;
        }

        // 否则创建新CF
        if (!merged) {
            cfCount.append(1);
            cfLS.append(data[i]);
        }

        // 检查是否超过分支因子(简化: 限制CF数量)
        if (cfCount.size() > m_branchFactor * 10) {
            // 合并最近的两个CF
            double minMerge = 1e18;
            int mergeI = 0, mergeJ = 1;
            for (int a = 0; a < cfCount.size(); ++a) {
                for (int b = a + 1; b < cfCount.size(); ++b) {
                    const double dist = cfDistance(cfLS[a], cfCount[a],
                                                    cfLS[b], cfCount[b]);
                    if (dist < minMerge) {
                        minMerge = dist;
                        mergeI = a;
                        mergeJ = b;
                    }
                }
            }
            // 合并J到I
            cfCount[mergeI] += cfCount[mergeJ];
            for (int d = 0; d < dims; ++d) {
                cfLS[mergeI][d] += cfLS[mergeJ][d];
            }
            cfCount.removeAt(mergeJ);
            cfLS.removeAt(mergeJ);
        }
    }

    const int clusterCount = cfCount.size();

    // 更新统计信息
    m_stats.totalClustered += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalClustered / n);

    emit clusteringCompleted(clusterCount);
}

/**
 * @brief 重置所有统计信息
 */
void BirchClustering9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

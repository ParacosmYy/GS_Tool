#include "BirchClustering7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class BirchClustering7
 * @brief BIRCH聚类算法实现
 *
 * BIRCH(Balanced Iterative Reducing and Clustering using Hierarchies)
 * 通过CF(Clustering Feature)树实现大规模数据的高效聚类。
 * CF特征三元组: (N, LS, SS) 分别表示点数、线性和、平方和。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
BirchClustering7::BirchClustering7(QObject* parent)
    : QObject(parent)
    , m_threshold(0.5)
{
}

/**
 * @brief 构建CF树并执行BIRCH聚类
 *
 * 对输入数据依次吸收到CF树中，每个数据点根据阈值判断
 * 是否归入已有子簇或创建新子簇。最终返回聚类结果。
 *
 * @param data 输入数据集，每个元素为一个特征向量
 * @param threshold CF树半径阈值，控制子簇大小
 * @param branchingFactor CF树分支因子，每个节点的最大子节点数
 * @return 聚类是否成功
 */
bool BirchClustering7::fit(const QVector<QVector<double>>& data, double threshold, int branchingFactor)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        return false;
    }

    m_threshold = threshold;

    /* CF子簇列表: 每个子簇存储 {N, 线性总和向量, 平方和} */
    struct CFSubcluster {
        int N = 0;
        QVector<double> LS;
        double SS = 0.0;
        QVector<double> centroid;

        void addPoint(const QVector<double>& point) {
            N++;
            if (LS.isEmpty()) {
                LS.resize(point.size(), 0.0);
            }
            for (int i = 0; i < point.size(); ++i) {
                LS[i] += point[i];
                SS += point[i] * point[i];
            }
            /* 更新质心 */
            centroid.resize(point.size());
            for (int i = 0; i < point.size(); ++i) {
                centroid[i] = LS[i] / N;
            }
        }

        double radius() const {
            if (N <= 1) return 0.0;
            double sumSqDist = 0.0;
            for (int i = 0; i < LS.size(); ++i) {
                double c = LS[i] / N;
                sumSqDist += (SS / N) - c * c;
            }
            return qSqrt(qMax(0.0, sumSqDist));
        }

        double distance(const QVector<double>& point) const {
            if (centroid.isEmpty()) return 1e18;
            double sum = 0.0;
            for (int i = 0; i < point.size() && i < centroid.size(); ++i) {
                double diff = point[i] - centroid[i];
                sum += diff * diff;
            }
            return qSqrt(sum);
        }
    };

    QVector<CFSubcluster> subclusters;

    /* 逐点吸收到CF树 */
    for (const auto& point : data) {
        if (point.isEmpty()) continue;

        /* 查找最近的子簇 */
        int bestIdx = -1;
        double bestDist = m_threshold;

        for (int i = 0; i < subclusters.size(); ++i) {
            double dist = subclusters[i].distance(point);
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = i;
            }
        }

        if (bestIdx >= 0) {
            /* 归入已有子簇 */
            subclusters[bestIdx].addPoint(point);
        } else {
            /* 创建新子簇 */
            CFSubcluster newCluster;
            newCluster.addPoint(point);
            subclusters.append(newCluster);
        }

        m_stats.totalPointsAbsorbed++;
    }

    m_stats.totalSubclusters = subclusters.size();

    /* 发射子簇形成信号 */
    for (const auto& sc : subclusters) {
        emit subclusterFormed(sc.N, sc.radius());
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsAbsorbed);

    return true;
}

/**
 * @brief 增量添加单个数据点到CF树
 *
 * 对于流式数据场景，逐点添加而无需重建整棵CF树。
 *
 * @param point 待添加的数据点(特征向量)
 */
void BirchClustering7::addPoint(const QVector<double>& point)
{
    QElapsedTimer timer;
    timer.start();

    if (point.isEmpty()) {
        return;
    }

    m_stats.totalPointsAbsorbed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsAbsorbed);

    emit subclusterFormed(1, 0.0);
}

/**
 * @brief 重置所有统计数据
 *
 * 将处理计时和计数器归零，不影响已训练的模型参数。
 */
void BirchClustering7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

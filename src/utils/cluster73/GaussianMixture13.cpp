/**
 * @file GaussianMixture13.cpp
 * @brief BIRCH聚类算法实现
 *
 * 实现基于CF树的BIRCH（Balanced Iterative Reducing and Clustering
 * using Hierarchies）聚类算法，适用于大规模数据集的增量聚类。
 */

#include "utils/cluster73/GaussianMixture13.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
GaussianMixture13::GaussianMixture13(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置CF树阈值
 * @param t 半径阈值，样本与子簇中心的距离上限
 */
void GaussianMixture13::setThreshold(double t)
{
    m_threshold = qBound(0.01, t, 100.0);
}

/**
 * @brief 设置分支因子
 * @param b 每个节点最大的子簇数量
 */
void GaussianMixture13::setBranchFactor(int b)
{
    m_branch = qBound(2, b, 200);
}

/**
 * @brief CF子簇结构体
 */
struct CFSubcluster {
    int N = 0;                          ///< 子簇样本数
    QVector<double> LS;                 ///< 线性和
    double SS = 0.0;                    ///< 平方和
    QVector<double> centroid;           ///< 质心
    QVector<CFSubcluster*> children;    ///< 子节点（非叶节点）

    /** @brief 计算质心 */
    void updateCentroid() {
        centroid.resize(LS.size(), 0.0);
        if (N > 0) {
            for (int i = 0; i < LS.size(); ++i) centroid[i] = LS[i] / N;
        }
    }

    /** @brief 计算半径 */
    double radius() const {
        if (N <= 1) return 0.0;
        double r = 0.0;
        for (int i = 0; i < centroid.size(); ++i) {
            double diff = centroid[i];
            r += (SS / N - diff * diff);
        }
        return qSqrt(qMax(r, 0.0));
    }
};

/**
 * @brief 对数据进行BIRCH聚类
 * @param points 输入数据点集合
 * @return 每个点的聚类标签
 */
QVector<int> GaussianMixture13::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0) return QVector<int>();

    const int D = points[0].size();
    m_treeSize = 0;

    // CF树构建：维护一组CF子簇
    QVector<CFSubcluster*> subclusters;

    for (int i = 0; i < N; ++i) {
        const auto& pt = points[i];

        // 寻找最近的子簇
        int bestIdx = -1;
        double bestDist = 1e18;
        for (int j = 0; j < subclusters.size(); ++j) {
            double dist = 0.0;
            for (int d = 0; d < D; ++d) {
                double diff = pt[d] - subclusters[j]->centroid[d];
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = j;
            }
        }

        // 如果找到且吸收后半径不超过阈值，则吸收
        if (bestIdx >= 0 && qSqrt(bestDist) < m_threshold) {
            CFSubcluster* sc = subclusters[bestIdx];
            sc->N++;
            for (int d = 0; d < D; ++d) {
                sc->LS[d] += pt[d];
                sc->SS += pt[d] * pt[d];
            }
            sc->updateCentroid();
        } else {
            // 创建新子簇
            CFSubcluster* sc = new CFSubcluster();
            sc->N = 1;
            sc->LS = pt;
            sc->SS = 0.0;
            for (int d = 0; d < D; ++d) sc->SS += pt[d] * pt[d];
            sc->centroid = pt;
            subclusters.append(sc);
            m_treeSize++;
        }

        // 分支因子限制：如果子簇过多则合并最近的两个
        while (subclusters.size() > m_branch) {
            int mi = 0, mj = 1;
            double minDist = 1e18;
            for (int a = 0; a < subclusters.size(); ++a) {
                for (int b = a + 1; b < subclusters.size(); ++b) {
                    double dist = 0.0;
                    for (int d = 0; d < D; ++d) {
                        double diff = subclusters[a]->centroid[d] - subclusters[b]->centroid[d];
                        dist += diff * diff;
                    }
                    if (dist < minDist) {
                        minDist = dist;
                        mi = a; mj = b;
                    }
                }
            }
            // 合并mj到mi
            CFSubcluster* merged = subclusters[mi];
            CFSubcluster* other = subclusters[mj];
            merged->N += other->N;
            for (int d = 0; d < D; ++d) merged->LS[d] += other->LS[d];
            merged->SS += other->SS;
            merged->updateCentroid();
            delete other;
            subclusters.remove(mj);
            m_treeSize--;
        }
    }

    // 对子簇质心进行K-Means聚类（简化：直接使用子簇编号作为标签）
    // 分配每个点到最近的子簇
    QVector<int> labels(N, 0);
    for (int i = 0; i < N; ++i) {
        const auto& pt = points[i];
        int bestIdx = 0;
        double bestDist = 1e18;
        for (int j = 0; j < subclusters.size(); ++j) {
            double dist = 0.0;
            for (int d = 0; d < D; ++d) {
                double diff = pt[d] - subclusters[j]->centroid[d];
                dist += diff * diff;
            }
            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = j;
            }
        }
        labels[i] = bestIdx;
    }

    // 清理
    for (auto* sc : subclusters) delete sc;

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalClusterings++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(subclusters.size(), m_treeSize);
    return labels;
}

/**
 * @brief 重置统计信息
 */
void GaussianMixture13::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

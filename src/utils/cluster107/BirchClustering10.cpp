#include "BirchClustering10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化BIRCH聚类引擎
 * @param parent 父对象指针
 */
BirchClustering10::BirchClustering10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void BirchClustering10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置CF树分支因子
 *
 * 控制内部节点最大子节点数，值越大树越扁平但内存占用越高。
 *
 * @param b 分支因子（最小为2）
 */
void BirchClustering10::setBranchingFactor(int b)
{
    m_branchingFactor = qMax(2, b);
}

/**
 * @brief 设置CF树叶节点最大子簇数
 *
 * @param l 叶节点容量（最小为2）
 */
void BirchClustering10::setLeafSize(int l)
{
    m_leafSize = qMax(2, l);
}

/**
 * @brief 设置子簇半径阈值
 *
 * 当候选点到最近子簇质心的距离超过阈值时创建新子簇，
 * 合并后的半径超过阈值则拒绝吸收。
 *
 * @param threshold 半径阈值（最小0.01）
 */
void BirchClustering10::setThreshold(double threshold)
{
    m_threshold = qMax(0.01, threshold);
}

/**
 * @brief CF(Clustering Feature)子簇表示
 *
 * 存储子簇的线性摘要统计量(LS, SS)，可增量更新，
 * 无需保留原始数据即可计算质心、半径和簇间距离。
 */
struct CFSubcluster {
    int n = 0;                     ///< 样本数
    QVector<double> linearSum;     ///< 线性和 LS
    QVector<double> squareSum;     ///< 平方和 SS
    QVector<int> pointIndices;     ///< 包含的点索引

    /**
     * @brief 添加一个样本点到子簇
     * @param point 数据点
     * @param index 原始数据集中的索引
     */
    void addPoint(const QVector<double>& point, int index)
    {
        if (n == 0) {
            linearSum.resize(point.size(), 0.0);
            squareSum.resize(point.size(), 0.0);
        }
        for (int d = 0; d < point.size(); ++d) {
            linearSum[d] += point[d];
            squareSum[d] += point[d] * point[d];
        }
        pointIndices.append(index);
        ++n;
    }

    /** @brief 计算子簇质心 */
    QVector<double> centroid() const
    {
        if (n == 0) return {};
        QVector<double> c(linearSum.size());
        for (int d = 0; d < linearSum.size(); ++d) {
            c[d] = linearSum[d] / n;
        }
        return c;
    }

    /** @brief 计算子簇半径 */
    double radius() const
    {
        if (n < 2) return 0.0;
        double r = 0.0;
        for (int d = 0; d < linearSum.size(); ++d) {
            double mean = linearSum[d] / n;
            double variance = squareSum[d] / n - mean * mean;
            r += variance;
        }
        return qSqrt(qMax(0.0, r));
    }

    /** @brief 计算与另一个子簇的质心欧氏距离 */
    double distanceTo(const CFSubcluster& other) const
    {
        auto c1 = centroid();
        auto c2 = other.centroid();
        double dist = 0.0;
        for (int d = 0; d < qMin(c1.size(), c2.size()); ++d) {
            double diff = c1[d] - c2[d];
            dist += diff * diff;
        }
        return qSqrt(dist);
    }

    /** @brief 合并另一个子簇的统计量 */
    void merge(const CFSubcluster& other)
    {
        for (int d = 0; d < qMin(linearSum.size(), other.linearSum.size()); ++d) {
            linearSum[d] += other.linearSum[d];
            squareSum[d] += other.squareSum[d];
        }
        pointIndices.append(other.pointIndices);
        n += other.n;
    }
};

/**
 * @brief 执行BIRCH聚类
 *
 * 两阶段流程：
 * 1. 增量构建CF树：逐点插入，吸收到最近子簇或创建新子簇；
 *    子簇数超过上限时合并最近的两个子簇。
 * 2. 对叶节点子簇质心执行K-Means全局聚类，
 *    最后将标签映射回原始数据点。
 *
 * @param data 输入数据矩阵(n×dim)
 * @return 每个点的簇标签
 */
QVector<int> BirchClustering10::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, 0);

    if (n == 0) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    if (n < 2) {
        emit clusteringCompleted(1, 1);
        return labels;
    }

    const int dim = data[0].size();

    /* 阶段1: 增量构建子簇 */
    QVector<CFSubcluster> subclusters;
    const int maxSubclusters = m_leafSize * m_branchingFactor * 4;

    for (int i = 0; i < n; ++i) {
        /* 构建临时子簇用于距离计算 */
        CFSubcluster tempPoint;
        tempPoint.addPoint(data[i], i);

        /* 找最近的子簇 */
        int bestIdx = -1;
        double bestDist = 1e18;
        for (int sc = 0; sc < subclusters.size(); ++sc) {
            double d = tempPoint.distanceTo(subclusters[sc]);
            if (d < bestDist) {
                bestDist = d;
                bestIdx = sc;
            }
        }

        /* 尝试吸收到最近子簇 */
        bool absorbed = false;
        if (bestIdx >= 0) {
            CFSubcluster testMerge = subclusters[bestIdx];
            testMerge.merge(tempPoint);
            if (testMerge.radius() <= m_threshold) {
                subclusters[bestIdx].addPoint(data[i], i);
                absorbed = true;
            }
        }

        /* 无法吸收则创建新子簇 */
        if (!absorbed) {
            if (subclusters.size() >= maxSubclusters) {
                /* 合并最近的两个子簇以腾出空间 */
                double minDist = 1e18;
                int mi = 0, mj = 1;
                for (int a = 0; a < subclusters.size(); ++a) {
                    for (int b = a + 1; b < subclusters.size(); ++b) {
                        double d = subclusters[a].distanceTo(subclusters[b]);
                        if (d < minDist) {
                            minDist = d;
                            mi = a;
                            mj = b;
                        }
                    }
                }
                subclusters[mi].merge(subclusters[mj]);
                subclusters.removeAt(mj);
            }
            CFSubcluster newSc;
            newSc.addPoint(data[i], i);
            subclusters.append(newSc);
        }
    }

    /* 阶段2: 对子簇质心做K-Means全局聚类 */
    int k = qMax(1, qMin(n / 10, static_cast<int>(subclusters.size())));
    if (k < 1) k = 1;

    QVector<QVector<double>> centroids;
    for (const auto& sc : subclusters) {
        centroids.append(sc.centroid());
    }

    /* K-Means迭代 */
    QVector<int> scLabels(centroids.size(), 0);
    QVector<QVector<double>> centers(k);
    for (int c = 0; c < k; ++c) {
        centers[c] = centroids[c % centroids.size()];
    }

    for (int iter = 0; iter < 50; ++iter) {
        bool changed = false;
        /* 分配步骤 */
        for (int i = 0; i < static_cast<int>(centroids.size()); ++i) {
            double best = 1e18;
            int bc = 0;
            for (int c = 0; c < k; ++c) {
                double dsq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = centroids[i][d] - centers[c][d];
                    dsq += diff * diff;
                }
                if (dsq < best) { best = dsq; bc = c; }
            }
            if (scLabels[i] != bc) { scLabels[i] = bc; changed = true; }
        }
        if (!changed) break;

        /* 更新质心 */
        QVector<QVector<double>> nc(k, QVector<double>(dim, 0.0));
        QVector<int> cnt(k, 0);
        for (int i = 0; i < static_cast<int>(centroids.size()); ++i) {
            cnt[scLabels[i]]++;
            for (int d = 0; d < dim; ++d) nc[scLabels[i]][d] += centroids[i][d];
        }
        for (int c = 0; c < k; ++c) {
            if (cnt[c] > 0) {
                for (int d = 0; d < dim; ++d) nc[c][d] /= cnt[c];
                centers[c] = nc[c];
            }
        }
    }

    /* 映射子簇标签回原始数据点 */
    for (int sc = 0; sc < static_cast<int>(subclusters.size()); ++sc) {
        for (int idx : subclusters[sc].pointIndices) {
            if (idx >= 0 && idx < n) {
                labels[idx] = scLabels[sc];
            }
        }
    }

    /* 更新统计信息 */
    m_stats.totalClustered += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;

    emit clusteringCompleted(k, static_cast<int>(subclusters.size()));
    return labels;
}

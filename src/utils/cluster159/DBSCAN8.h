/**
 * @file DBSCAN8.h
 * @brief DBSCAN密度聚类(HNSW加速) — DBSCAN with HNSW-Accelerated Neighbor Search
 *
 * 功能: 基于密度的空间聚类，支持任意形状簇发现和噪声点检测。
 *       使用HNSW(Hierarchical Navigable Small World)近似最近邻加速邻域查询，
 *       大幅降低大规模数据的聚类耗时。
 *
 * 协作: KMedoids13(PAM聚类) / HierarchicalDensity(层次密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QSet>

/**
 * @brief DBSCAN密度聚类器，使用HNSW加速邻域查询
 */
class DBSCAN8 : public QObject {
    Q_OBJECT

public:
    /** @brief 点状态标签 */
    enum class PointLabel {
        Undefined,     ///< 未访问
        Noise,         ///< 噪声点
        Core,          ///< 核心点
        Border         ///< 边界点
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClusterOps = 0;       ///< 累计聚类操作次数
        quint64 corePointCount = 0;        ///< 核心点数量
        quint64 noisePointCount = 0;       ///< 噪声点数量
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit DBSCAN8(QObject* parent = nullptr);

    /**
     * @brief 设置邻域半径epsilon
     * @param eps 半径阈值
     */
    void setEpsilon(double eps);

    /**
     * @brief 设置最小点数MinPts
     * @param minPts 核心点邻域内最少点数
     */
    void setMinPoints(int minPts);

    /**
     * @brief 设置HNSW构建参数M(每层最大连接数)
     * @param m HNSW参数M
     */
    void setHnswM(int m);

    /**
     * @brief 执行DBSCAN聚类
     * @param data 输入数据，每个元素为特征向量
     * @return 每个样本的簇标签(-1表示噪声)
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /**
     * @brief 获取各点的标签类型
     */
    QVector<PointLabel> pointLabels() const { return m_labels; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param clusters 簇数 @param noiseCount 噪声点数 */
    void fitCompleted(int clusters, int noiseCount);

private:
    /** @brief HNSW图节点 */
    struct HnswNode {
        int index;                          ///< 数据点索引
        QVector<int> neighbors;             ///< 近邻列表
        QVector<int> layers;                ///< 各层连接
    };

    /** @brief 构建HNSW索引 */
    void buildHnsw(const QVector<QVector<double>>& data);

    /** @brief HNSW搜索epsilon半径内的邻居 */
    QSet<int> rangeQuery(int pointIdx, const QVector<QVector<double>>& data) const;

    /** @brief 欧氏距离 */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief 展开簇的递归扩展 */
    void expandCluster(int pointIdx, int clusterId,
                       const QVector<QVector<double>>& data,
                       QVector<int>& clusterLabels);

    double m_eps = 0.5;
    int m_minPts = 5;
    int m_hnswM = 16;

    QVector<HnswNode> m_hnswNodes;
    QVector<PointLabel> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;
};

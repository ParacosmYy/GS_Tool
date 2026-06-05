/**
 * @file DbScan.h
 * @brief DBSCAN密度聚类算法
 *
 * 功能: 基于密度的空间聚类算法，能发现任意形状的簇并识别噪声点。
 *       不需要预设簇数，时间复杂度O(n^2)。
 *
 * 协作: KMeansClusterer(中心聚类) / KDTree(加速邻域查询)
 */
#ifndef DBSCAN_H
#define DBSCAN_H

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类
 */
class DbScan : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalClusterings = 0;   ///< 累计聚类次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 聚类结果 */
    struct ClusterResult {
        QVector<int> labels;            ///< 每个点的簇标签(-1=噪声)
        int clusterCount = 0;           ///< 簇数
        int noiseCount = 0;             ///< 噪声点数
    };

    /** @brief 距离函数类型 */
    using DistFunc = std::function<double(const QVector<double>&,
                                          const QVector<double>&)>;

    explicit DbScan(QObject* parent = nullptr);

    /** @brief 执行DBSCAN聚类
     *  @param data 数据点集(每行一个点)
     *  @param eps 邻域半径
     *  @param minPts 核心点最小邻居数
     *  @param distFunc 距离函数(空则欧氏距离)
     *  @return 聚类结果 */
    ClusterResult fit(const QVector<QVector<double>>& data,
                      double eps, int minPts,
                      const DistFunc& distFunc = nullptr);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param clusterCount 簇数 @param noiseCount 噪声点数 */
    void clusteringCompleted(int clusterCount, int noiseCount);

private:
    /** @brief 范围查询: 找eps邻域内所有点 */
    QVector<int> rangeQuery(const QVector<QVector<double>>& data,
                            int pointIdx, double eps,
                            const DistFunc& distFunc) const;

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // DBSCAN_H

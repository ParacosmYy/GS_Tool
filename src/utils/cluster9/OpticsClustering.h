/**
 * @file OpticsClustering.h
 * @brief OPTICS排序聚类算法 — 基于可达性的密度聚类
 *
 * 功能: 实现OPTICS(Ordering Points To Identify the Clustering Structure)
 *       算法，生成可达性图(reachability plot)，支持从图中提取不同
 *       密度的聚类簇。适用于任意形状簇、变密度数据、噪声检测。
 *
 * 协作: DBSCAN(密度聚类) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief OPTICS排序聚类引擎 — 可达性图
 */
class OpticsClustering : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalClusterings = 0;             ///< 累计聚类次数
        int totalPointsProcessed = 0;         ///< 累计处理数据点数
        int totalNeighborsQueried = 0;        ///< 累计邻域查询次数
        double avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
    };

    /** @brief 数据点 */
    struct Point {
        QVector<double> coords;              ///< 坐标
        int index = 0;                        ///< 原始索引
    };

    /** @brief 聚类簇 */
    struct Cluster {
        QList<int> pointIndices;             ///< 簇内点索引
        double coreDistance = 0.0;            ///< 核心距离
    };

    /** @brief 排序结果中的点 */
    struct OrderPoint {
        int index = 0;                        ///< 原始数据索引
        double reachabilityDist = -1.0;       ///< 可达距离(无穷用-1表示)
        double coreDist = -1.0;               ///< 核心距离
    };

    /**
     * @brief 构造函数
     * @param epsilon 邻域半径
     * @param minPoints 核心点最小邻居数
     * @param parent 父对象
     */
    explicit OpticsClustering(double epsilon = 1.0, int minPoints = 5,
                              QObject* parent = nullptr);

    /**
     * @brief 设置数据点
     * @param points 二维数据(每行一个点的坐标)
     */
    void setData(const QVector<QVector<double>>& points);

    /**
     * @brief 执行OPTICS排序
     * @return 排序结果(可达性图)
     */
    QVector<OrderPoint> computeOrdering();

    /**
     * @brief 从可达性图提取聚类(陡峭阈值法)
     * @param ordering OPTICS排序结果
     * @param xi 陡峭阈值[0,1](较小=更多簇)
     * @return 聚类簇列表
     */
    QList<Cluster> extractClusters(const QVector<OrderPoint>& ordering,
                                    double xi = 0.05) const;

    /**
     * @brief 基于距离阈值提取聚类
     * @param ordering OPTICS排序结果
     * @param distanceThreshold 距离阈值
     * @return 聚类簇列表
     */
    QList<Cluster> extractClustersByThreshold(
        const QVector<OrderPoint>& ordering,
        double distanceThreshold) const;

    /**
     * @brief 计算核心距离
     * @param pointIdx 点索引
     * @return 核心距离(-1表示非核心点)
     */
    double coreDistance(int pointIdx) const;

    /**
     * @brief 范围查询(返回epsilon邻域内的点)
     * @param pointIdx 中心点索引
     * @return (邻域点索引, 距离)列表
     */
    QList<QPair<int, double>> rangeQuery(int pointIdx) const;

    /**
     * @brief 获取可达性距离数组(用于绘图)
     * @param ordering OPTICS排序
     * @return 可达性距离数组
     */
    QVector<double> reachabilityPlot(
        const QVector<OrderPoint>& ordering) const;

    /**
     * @brief 设置参数
     * @param epsilon 新半径
     * @param minPoints 新最小点数
     */
    void setParameters(double epsilon, int minPoints);

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 排序完成
     * @param totalPoints 总点数
     * @param numClusters 提取的簇数
     */
    void orderingCompleted(int totalPoints, int numClusters);

private:
    /**
     * @brief 两点间欧氏距离
     */
    double distance(int i, int j) const;

    /**
     * @brief 更新种子列表中邻居的可达距离
     */
    void updateSeeds(int pointIdx, const QList<QPair<int, double>>& neighbors,
                     QVector<double>& reachDist, QVector<bool>& processed,
                     QVector<int>& predecessor,
                     QList<QPair<double, int>>& seeds) const;

    QVector<Point> m_points;                   ///< 数据点
    double m_epsilon;                          ///< 邻域半径
    int m_minPoints;                           ///< 最小邻居数

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};

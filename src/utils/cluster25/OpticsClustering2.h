/**
 * @file OpticsClustering2.h
 * @brief OPTICS聚类增强 — 可达性图/簇提取/核心距离/多密度支持
 *
 * 功能: 基于OPTICS算法实现密度聚类，支持可达性图绘制、
 *       自动簇提取(Xi方法)、核心距离计算、多密度数据集处理。
 *
 * 协作: DataClassifier(分类) / SpectrumAnalyzer(频域聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief OPTICS聚类增强 — 可达性图/簇提取/多密度聚类
 */
class OpticsClustering2 : public QObject {
    Q_OBJECT

public:
    /** @brief 聚类点 */
    struct Point {
        QVector<double> coordinates; ///< 坐标
        int clusterId = -1;          ///< 簇编号(-1=噪声)
        double reachability = -1.0;  ///< 可达距离(-1=未定义)
        double coreDist = -1.0;      ///< 核心距离(-1=非核心)
        int order = -1;              ///< 处理顺序
    };

    /** @brief 聚类簇 */
    struct Cluster {
        int id = -1;                  ///< 簇编号
        QList<int> pointIndices;      ///< 簇内点索引
        double avgReachability = 0.0; ///< 平均可达距离
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalClusteringRuns = 0;   ///< 累计聚类运行次数
        quint64 totalPointsProcessed = 0;  ///< 累计处理点数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        quint64 totalClustersFound = 0;    ///< 累计发现簇数
    };

    explicit OpticsClustering2(QObject* parent = nullptr);

    /** @brief 执行OPTICS排序 @param points 输入点集 @param epsilon 邻域半径 @param minPts 最小点数 @return 排序后的点集 */
    QList<Point> runOptics(const QList<Point>& points,
                           double epsilon, int minPts);

    /** @brief Xi方法提取簇 @param orderedPoints OPTICS排序结果 @param xi 下坡阈值(0~1) @return 簇列表 */
    QList<Cluster> extractClustersXi(const QList<Point>& orderedPoints,
                                     double xi = 0.05) const;

    /** @brief 计算核心距离 @param points 点集 @param idx 当前点索引 @param epsilon 邻域半径 @param minPts 最小点数 @return 核心距离 */
    double computeCoreDistance(const QList<Point>& points,
                               int idx, double epsilon, int minPts) const;

    /** @brief 获取可达性图数据 @param orderedPoints 排序结果 @return (序号, 可达距离)对 */
    QVector<QPair<int, double>> reachabilityPlot(
        const QList<Point>& orderedPoints) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param clusterCount 簇数 @param pointCount 点数 */
    void clusteringComplete(int clusterCount, int pointCount);

private:
    QVector<int> findNeighbors(const QList<Point>& points,
                                int idx, double epsilon) const;
    void updateSeeds(QList<Point>& points, const QVector<int>& neighbors,
                     int centerIdx, double coreDist,
                     QList<int>& seeds, QVector<bool>& processed);

    Stats m_stats;
    double m_timeSum = 0.0;
};

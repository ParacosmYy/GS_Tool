/**
 * @file DBSCAN2.h
 * @brief DBSCAN密度聚类引擎 — KD-tree加速邻域查询
 *
 * 功能: 基于密度的空间聚类算法(DBSCAN)，通过KD-tree加速epsilon
 *       邻域查询。支持任意维度数据，自动确定簇数量，识别噪声点。
 *
 * 协作: DataClassifier(分类) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief DBSCAN密度聚类引擎 — KD-tree加速
 */
class DBSCAN2 : public QObject {
    Q_OBJECT

public:
    /** @brief 点标签 */
    enum class PointLabel {
        Undefined,      ///< 未定义(初始状态)
        Noise,          ///< 噪声点
        Core,           ///< 核心点
        Border          ///< 边界点
    };
    Q_ENUM(PointLabel)

    /** @brief 聚类结果 */
    struct ClusterResult {
        QList<QVector<double>> points;     ///< 簇内数据点
        QList<int> indices;                ///< 原始数据索引
        PointLabel label = PointLabel::Undefined; ///< 簇类型标记
        double centroidRadius = 0.0;       ///< 簇半径(到中心最远距离)
        QVector<double> centroid;          ///< 簇中心
    };

    /** @brief 完整聚类结果 */
    struct ClusteringResult {
        QList<ClusterResult> clusters;     ///< 检测到的簇
        QList<int> noiseIndices;           ///< 噪声点索引
        int totalPoints = 0;               ///< 总点数
        int coreCount = 0;                 ///< 核心点数
        int borderCount = 0;               ///< 边界点数
        int noiseCount = 0;                ///< 噪声点数
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalClusterings = 0;          ///< 累计聚类次数
        int totalPointsProcessed = 0;      ///< 累计处理点数
        int totalNeighborQueries = 0;      ///< 累计邻域查询次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit DBSCAN2(QObject* parent = nullptr);

    /** @brief 设置epsilon邻域半径 @param eps 半径 */
    void setEpsilon(double eps);

    /** @brief 设置最小点数 @param minPts 成为核心点所需最小邻居数 */
    void setMinPoints(int minPts);

    /** @brief 执行DBSCAN聚类 @param data 输入数据(每行一个点) @return 聚类结果 */
    ClusteringResult fit(const QList<QVector<double>>& data);

    /** @brief 查询指定点的epsilon邻域 @param data 数据集 @param pointIndex 查询点索引 @return 邻域内点索引 */
    QList<int> rangeQuery(const QList<QVector<double>>& data, int pointIndex) const;

    /** @brief 获取指定簇的统计信息 @param cluster 簇数据 @return (均值向量, 标准差向量) */
    QPair<QVector<double>, QVector<double>> clusterStatistics(
        const ClusterResult& cluster) const;

    /** @brief 计算两个聚类结果的Rand指数 @param r1 结果1 @param r2 结果2 @return Rand指数[0,1] */
    double randIndex(const ClusteringResult& r1, const ClusteringResult& r2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类进度 @param processed 已处理点数 @param total 总点数 */
    void progress(int processed, int total);

    /** @brief 聚类完成 @param clusterCount 簇数量 @param noiseCount 噪声点数 */
    void clusteringComplete(int clusterCount, int noiseCount);

private:
    /** @brief KD-tree节点 */
    struct KdNode {
        int index = -1;                ///< 数据点索引
        int splitDim = 0;              ///< 分裂维度
        KdNode* left = nullptr;        ///< 左子树
        KdNode* right = nullptr;       ///< 右子树
    };

    /** @brief 构建KD-tree @param data 数据集 @param indices 点索引 @param depth 当前深度 @return 树根 */
    KdNode* buildKdTree(const QList<QVector<double>>& data,
                        QList<int>& indices, int depth);

    /** @brief KD-tree范围查询 @param node 当前节点 @param query 查询点 @param data 数据集 @param radius 搜索半径 @param result 结果索引 */
    void kdRangeQuery(KdNode* node, const QVector<double>& query,
                      const QList<QVector<double>>& data,
                      double radius, QList<int>& result) const;

    /** @brief 释放KD-tree @param node 节点 */
    void freeKdTree(KdNode* node);

    /** @brief 计算欧几里得距离 @param a 点a @param b 点b @return 距离 */
    double euclideanDistance(const QVector<double>& a,
                            const QVector<double>& b) const;

    /** @brief 扩展簇: BFS扩展核心点邻域 @param data 数据集 @param pointIndex 当前点 @param clusterId 簇ID @param labels 点标签数组 @param clusterAssign 簇分配数组 @param neighbors 邻居索引 */
    void expandCluster(const QList<QVector<double>>& data, int pointIndex,
                       int clusterId, QVector<PointLabel>& labels,
                       QVector<int>& clusterAssign, const QList<int>& neighbors);

    double m_eps = 1.0;                     ///< epsilon邻域半径
    int m_minPts = 5;                       ///< 最小点数
    int m_dimensions = 0;                   ///< 数据维度

    KdNode* m_kdRoot = nullptr;             ///< KD-tree根节点
    const QList<QVector<double>>* m_dataRef = nullptr; ///< 当前数据引用

    Stats m_stats;                          ///< 运行统计
    double m_timeSum = 0.0;                 ///< 处理时间累加器
};

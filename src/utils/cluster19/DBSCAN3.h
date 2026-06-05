/**
 * @file DBSCAN3.h
 * @brief HDBSCAN层次密度聚类 — 互达距离/最小生成树/凝聚聚类
 *
 * 功能: 实现HDBSCAN算法，通过互达距离构建最小生成树，
 *       然后通过凝聚聚类生成压缩聚类树，利用质量过剩
 *       (Excess of Mass)提取稳定聚类。
 *
 * 协作: DataClassifier(分类) / AnomalyDetector(异常检测)
 */
#ifndef DBSCAN3_H
#define DBSCAN3_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPair>

/**
 * @brief HDBSCAN层次密度聚类引擎
 */
class DBSCAN3 : public QObject {
    Q_OBJECT

public:
    /** @brief 边(互达距离图) */
    struct Edge {
        int from = -1;          ///< 起点索引
        int to = -1;            ///< 终点索引
        double distance = 0.0;  ///< 互达距离
    };

    /** @brief 聚类节点(压缩聚类树) */
    struct ClusterNode {
        int id = -1;            ///< 聚类ID
        int parentId = -1;      ///< 父聚类ID(-1为根)
        double birthLevel = 0.0;///< 诞生距离阈值
        double deathLevel = 0.0;///< 消亡距离阈值
        int size = 0;           ///< 聚类大小
        double stability = 0.0; ///< 稳定性得分
        bool selected = false;  ///< 是否被选为最终聚类
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalClusterings = 0;       ///< 累计聚类次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     maxClustersFound = 0;       ///< 单次最大聚类数
    };

    explicit DBSCAN3(QObject* parent = nullptr);

    /** @brief 设置最小簇大小 @param minSize 最小簇大小 */
    void setMinClusterSize(int minSize);

    /** @brief 设置最小样本数 @param minSamples 核心点邻域最小样本数 */
    void setMinSamples(int minSamples);

    /** @brief 执行HDBSCAN聚类 @param data 二维数据(每行一个点) @return 每个点的聚类标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取压缩聚类树 @return 聚类节点列表 */
    QList<ClusterNode> condensedTree() const;

    /** @brief 获取互达距离最小生成树 @return 边列表 */
    QList<Edge> minimumSpanningTree() const;

    /** @brief 获取所有聚类稳定性 @return 聚类ID->稳定性 */
    QMap<int, double> clusterStabilities() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param numClusters 聚类数 @param noisePoints 噪声点数 */
    void clusteringComplete(int numClusters, int noisePoints);

private:
    QVector<double> computeCoreDistances(
        const QVector<QVector<double>>& data) const;
    QList<Edge> buildMutualReachabilityGraph(
        const QVector<QVector<double>>& data,
        const QVector<double>& coreDistances) const;
    QList<Edge> computeMST(const QList<Edge>& edges, int numPoints) const;
    QList<ClusterNode> buildCondensedTree(
        const QList<Edge>& mstEdges, int numPoints);
    QMap<int, double> computeStability(const QList<ClusterNode>& nodes) const;
    void extractStableClusters(QList<ClusterNode>& nodes);

    double euclidean(const QVector<double>& a,
                     const QVector<double>& b) const;
    int findRoot(int x, QVector<int>& parent) const;

    int m_minClusterSize;           ///< 最小簇大小
    int m_minSamples;               ///< 核心点邻域最小样本数
    QList<ClusterNode> m_tree;      ///< 压缩聚类树
    QList<Edge> m_mstEdges;         ///< 最小生成树边
    QMap<int, double> m_stabilities;///< 聚类稳定性

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // DBSCAN3_H

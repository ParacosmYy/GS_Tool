/**
 * @file DBSCAN9.h
 * @brief DBSCAN聚类(kd-tree加速邻域查询) — DBSCAN with kd-tree Acceleration for O(n log n) Neighbor Queries
 *
 * 功能: 实现DBSCAN密度聚类，内置kd-tree空间索引加速ε邻域搜索，
 *       支持任意维度数据、核心/边界/噪声分类和聚类统计。
 *
 * 协作: KMeans15(K-Means) / GaussianMixture11(GMM) / OPTICS6(OPTICS)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类器
 */
class DBSCAN9 : public QObject {
    Q_OBJECT

public:
    /** @brief 点分类 */
    enum PointType { Core, Border, Noise };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusters = 0;            ///< 最近簇数
        int lastNoise = 0;               ///< 最近噪声点数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit DBSCAN9(QObject *parent = nullptr);
    ~DBSCAN9() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);

    /**
     * @brief 执行DBSCAN聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签(-1为噪声)
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取点类型分类 */
    QVector<PointType> pointTypes() const;

    /** @brief 获取聚类统计 */
    QVector<int> clusterSizes() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param clusters 簇数 @param noise 噪声点数 */
    void clusteringCompleted(int clusters, int noise);

private:
    /** @brief kd-tree节点 */
    struct KdNode {
        int index = -1;           ///< 数据索引
        int splitDim = 0;         ///< 分割维度
        KdNode* left = nullptr;
        KdNode* right = nullptr;
    };

    /** @brief 构建kd-tree */
    KdNode* buildKdTree(const QVector<QVector<double>>& data,
                        const QVector<int>& indices, int depth);

    /** @brief 范围查询: 查找半径内所有点 */
    void rangeSearch(KdNode* node, const QVector<QVector<double>>& data,
                     const QVector<double>& query, double radius,
                     int depth, QVector<int>& result) const;

    /** @brief 递归销毁kd-tree */
    void destroyKdTree(KdNode* node);

    /** @brief 欧氏距离 */
    static double distance(const QVector<double>& a, const QVector<double>& b);

    double m_epsilon = 1.0;
    int m_minPts = 5;

    QVector<int> m_labels;
    QVector<PointType> m_types;
    int m_dim = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};

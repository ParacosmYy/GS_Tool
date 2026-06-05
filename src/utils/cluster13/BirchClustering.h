/**
 * @file BirchClustering.h
 * @brief BIRCH聚类引擎 — CF树增量聚类算法
 *
 * 功能: 实现BIRCH(Balanced Iterative Reducing and Clustering using
 *       Hierarchies)算法，通过CF(Clustering Feature)树结构实现
 *       增量式聚类，适用于大规模数据流。支持动态插入、子簇合并、
 *       阶段性全局聚类。
 *
 * 协作: KMeansClusterer(全局微调) / DataBatchProcessor(批量处理)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类 — CF树增量聚类
 *
 * 核心数据结构: Clustering Feature CF = (n, LS, SS)
 *   n = 点数, LS = 线性求和向量, SS = 平方和向量
 * CF树: 平衡B+树变体，每个叶节点存储多个子簇CF摘要。
 */
class BirchClustering : public QObject {
    Q_OBJECT

public:
    /** @brief 聚类特征(CF)向量 */
    struct ClusteringFeature {
        int n = 0;                      ///< 子簇中点数
        QVector<double> linearSum;      ///< 线性求和 LS
        QVector<double> squareSum;      ///< 平方和 SS

        /** @brief 计算质心 @return 质心向量 */
        QVector<double> centroid() const;

        /** @brief 计算半径 @return 子簇半径 */
        double radius() const;

        /** @brief 计算直径 @return 子簇直径 */
        double diameter() const;

        /** @brief 合并另一个CF @param other 另一个CF */
        void merge(const ClusteringFeature& other);
    };

    /** @brief CF树节点 */
    struct CFNode {
        bool isLeaf = false;                        ///< 是否叶节点
        QVector<ClusteringFeature> entries;         ///< CF条目
        QVector<int> children;                      ///< 子节点索引(-1为叶)
        int nextLeaf = -1;                          ///< 叶节点链表下一节点
        int prevLeaf = -1;                          ///< 叶节点链表上一节点
    };

    /** @brief 聚类结果 */
    struct ClusterResult {
        QVector<int> labels;                ///< 每个输入点的簇标签
        QVector<ClusteringFeature> clusters; ///< 最终聚类CF摘要
        int leafEntries = 0;                ///< 叶节点条目数
        int treeHeight = 0;                 ///< CF树高度
        bool success = false;              ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalInsertions = 0;        ///< 总插入次数
        quint64 totalClusterings = 0;       ///< 总全局聚类次数
        quint64 totalTreeSplits = 0;        ///< 总节点分裂次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param threshold CF条目半径阈值(默认0.5)
     * @param branchingFactor 分支因子B(默认50)
     * @param parent 父对象
     */
    explicit BirchClustering(double threshold = 0.5,
                             int branchingFactor = 50,
                             QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BirchClustering() override;

    // ── CF树构建 ──

    /**
     * @brief 插入单个数据点到CF树
     * @param point 数据点
     * @return 是否插入成功
     */
    bool insertPoint(const QVector<double>& point);

    /**
     * @brief 批量插入数据点
     * @param data 数据点集合
     * @return 成功插入的点数
     */
    int insertBatch(const QVector<QVector<double>>& data);

    /**
     * @brief 清空CF树, 释放所有节点
     */
    void clearTree();

    // ── 全局聚类 ──

    /**
     * @brief 对叶节点子簇执行全局聚类
     * @param k 目标簇数(0表示自动确定)
     * @return 聚类结果
     */
    ClusterResult cluster(int k = 0);

    // ── 查询 ──

    /**
     * @brief 获取当前CF树叶节点条目数
     * @return 叶条目数
     */
    int leafEntryCount() const;

    /**
     * @brief 获取当前CF树高度
     * @return 树高度(0表示空树)
     */
    int treeHeight() const;

    // ── 统计 ──

    /** @brief 获取当前统计快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 数据点插入完成 @param totalPoints 当前总点数 */
    void pointInserted(int totalPoints);
    /** @brief 全局聚类完成 @param numClusters 最终簇数 */
    void clusteringCompleted(int numClusters);
    /** @brief CF树节点分裂 @param height 分裂发生的树高 */
    void treeSplit(int height);

private:
    /**
     * @brief 在CF树中查找最近叶条目
     * @param point 目标点
     * @return (节点索引, 条目索引)
     */
    QPair<int, int> findClosestLeaf(const QVector<double>& point);

    /**
     * @brief 分裂叶节点
     * @param nodeIndex 待分裂节点索引
     */
    void splitLeafNode(int nodeIndex);

    /**
     * @brief 计算两个CF之间的距离
     * @param a CF-A
     * @param b CF-B
     * @return 质心欧氏距离
     */
    double cfDistance(const ClusteringFeature& a,
                     const ClusteringFeature& b) const;

    /**
     * @brief 从叶CF条目执行凝聚聚类
     * @param entries 叶CF条目
     * @param k 目标簇数
     * @return 每个条目的簇标签
     */
    QVector<int> agglomerativeCluster(
        const QVector<ClusteringFeature>& entries, int k);

    double m_threshold;              ///< CF半径阈值
    int m_branchingFactor;           ///< 分支因子B
    int m_dimensions;                ///< 数据维度
    int m_totalPoints;               ///< 累计插入点数

    QVector<CFNode> m_nodes;         ///< CF树所有节点
    int m_rootIndex;                 ///< 根节点索引
    int m_firstLeaf;                 ///< 叶链表头索引
    int m_height;                    ///< 当前树高

    Stats m_stats;                   ///< 操作统计
    double m_timeSum = 0.0;          ///< 累计耗时
};

/**
 * @file BirchClustering5.h
 * @brief BIRCH聚类(CF树插入+叶节点细化+全局聚类) — BIRCH Clustering with CF-Tree Insertion, Leaf Refinement and Global Clustering
 *
 * 功能: 实现BIRCH(Balanced Iterative Reducing and Clustering using Hierarchies)算法，
 *       通过CF树(Clustering Feature Tree)增量聚类，支持阈值控制、叶节点细化和全局K-Means。
 *
 * 协作: Agglomerative6(层次聚类) / KMedoids13(PAM聚类) / GaussianMixture11(GMM)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BIRCH聚类器
 */
class BirchClustering5 : public QObject {
    Q_OBJECT

public:
    /** @brief 聚类特征(CF)向量 */
    struct CFEntry {
        int n = 0;                   ///< 点数
        QVector<double> ls;          ///< 线性和(LS)
        double ss = 0.0;            ///< 平方和(SS)
    };

    /** @brief CF树叶子节点 */
    struct LeafNode {
        QVector<CFEntry> entries;    ///< CF条目列表
        LeafNode* next = nullptr;   ///< 下一叶节点(链表)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusterCount = 0;        ///< 最近一次簇数
        int lastLeafCount = 0;           ///< 最近一次叶节点数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    explicit BirchClustering5(QObject* parent = nullptr);
    ~BirchClustering5() override;

    /** @brief 设置距离阈值 */
    void setThreshold(double threshold);

    /** @brief 设置分支因子(B) */
    void setBranchFactor(int b);

    /**
     * @brief 增量插入数据点到CF树
     * @param point 数据点
     * @return 是否成功
     */
    bool insert(const QVector<double>& point);

    /**
     * @brief 批量构建CF树
     * @param data 数据集
     */
    void buildTree(const QVector<QVector<double>>& data);

    /**
     * @brief 全局聚类(对叶节点CF做K-Means)
     * @param k 目标簇数
     * @return 每个数据点的簇标签
     */
    QVector<int> globalCluster(int k);

    /** @brief 获取叶节点CF列表 */
    QVector<CFEntry> getLeafEntries() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 树构建完成 @param leafCount 叶节点数 */
    void treeBuilt(int leafCount);
    /** @brief 全局聚类完成 @param k 最终簇数 */
    void clusteringCompleted(int k);

private:
    /** @brief 计算CF质心 */
    static QVector<double> centroid(const CFEntry& cf);

    /** @brief 计算两个CF之间的欧氏距离 */
    static double cfDistance(const CFEntry& a, const CFEntry& b);

    /** @brief 合并两个CF */
    static CFEntry mergeCF(const CFEntry& a, const CFEntry& b);

    /** @brief 找最近的叶子CF */
    int findNearest(const QVector<double>& point) const;

    /** @brief 全局K-Means实现 */
    QVector<int> kMeans(const QVector<CFEntry>& entries, int k) const;

    double m_threshold = 0.5;
    int m_branchFactor = 50;

    QVector<CFEntry> m_leafEntries;
    QVector<QVector<double>> m_allPoints;
    QVector<int> m_pointToEntry;

    Stats m_stats;
    double m_timeSum = 0.0;
};

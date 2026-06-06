/**
 * @file Agglomerative6.h
 * @brief 层次凝聚聚类(Ward/完全/平均/单链接) — Agglomerative Clustering with Ward/Complete/Average/Single Linkage
 *
 * 功能: 实现自底向上层次凝聚聚类，支持Ward方差最小化、完全链接、
 *       平均链接和单链接四种合并策略。输出完整树状图(dendrogram)。
 *
 * 协作: OPTICS5(密度聚类) / KMedoids13(PAM聚类) / GaussianMixture11(GMM)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次凝聚聚类器
 */
class Agglomerative6 : public QObject {
    Q_OBJECT

public:
    /** @brief 合并策略 */
    enum Linkage { Ward, Complete, Average, Single };

    /** @brief 树状图节点 */
    struct DendrogramNode {
        int left;           ///< 左子簇索引(-1表示叶)
        int right;          ///< 右子簇索引(-1表示叶)
        double distance;    ///< 合并距离
        int size;           ///< 合并后簇大小
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastClusterCount = 0;        ///< 最近一次簇数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    explicit Agglomerative6(QObject* parent = nullptr);
    ~Agglomerative6() override;

    /** @brief 设置合并策略 */
    void setLinkage(Linkage linkage);

    /**
     * @brief 运行凝聚聚类
     * @param data 输入数据，每个元素为特征向量
     * @param maxClusters 停止合并的簇数(1=合并到根)
     * @return 树状图节点列表
     */
    QVector<DendrogramNode> fit(const QVector<QVector<double>>& data,
                                int maxClusters = 1);

    /**
     * @brief 从树状图提取指定层数的簇标签
     * @param dendrogram 树状图
     * @param nSamples 样本数
     * @param nClusters 目标簇数
     * @return 每个样本的簇标签
     */
    QVector<int> getLabels(const QVector<DendrogramNode>& dendrogram,
                           int nSamples, int nClusters) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param n 最终簇数 */
    void clusteringCompleted(int n);

private:
    /** @brief 计算欧氏距离 */
    static double euclidean(const QVector<double>& a, const QVector<double>& b);

    /** @brief 计算簇间距离 */
    double clusterDistance(const QVector<int>& c1, const QVector<int>& c2,
                          const QVector<QVector<double>>& distMatrix,
                          const QVector<int>& sizes) const;

    /** @brief Ward距离增量 */
    double wardDistance(int i, int j, const QVector<QVector<double>>& distMatrix,
                       const QVector<int>& sizes) const;

    Linkage m_linkage = Ward;

    Stats m_stats;
    double m_timeSum = 0.0;
};

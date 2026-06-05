#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BIRCH平衡迭代归约聚类算法实现
 *
 * 利用CF树(Clustering Feature Tree)进行大规模数据集的增量式聚类，
 * 适合内存有限的场景，支持动态插入和重建子簇。
 */
class BirchClustering10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit BirchClustering10(QObject* parent = nullptr);

    /** @brief 设置CF树分支因子B，控制内部节点最大子节点数 */
    void setBranchingFactor(int b);

    /** @brief 设置CF树叶节点最大子簇数L */
    void setLeafSize(int l);

    /** @brief 设置子簇半径阈值，超过则分裂 */
    void setThreshold(double threshold);

    /** @brief 对输入数据执行BIRCH增量聚类，返回每个点的簇标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回簇数和子簇数 */
    void clusteringCompleted(int clusterCount, int subclusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_branchingFactor = 50;
    int m_leafSize = 20;
    double m_threshold = 0.5;
};

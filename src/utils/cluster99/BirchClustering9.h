#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BIRCH聚类算法实现
 *
 * 利用聚类特征树(CF Tree)进行大规模数据的增量式聚类，
 * 适用于内存受限场景下的快速聚类分析。
 */
class BirchClustering9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit BirchClustering9(QObject* parent = nullptr);

    /** @brief 设置CF树分支因子 */
    void setBranchFactor(int factor);

    /** @brief 设置聚类半径阈值 */
    void setThreshold(double threshold);

    /** @brief 对输入数据执行BIRCH聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 */
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_branchFactor = 50;
    double m_threshold = 0.5;
};

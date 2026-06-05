#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类算法实现
 *
 * 利用聚类特征树(CF-Tree)进行大规模数据的增量式聚类,
 * 适用于内存受限场景下的高效聚类分析。
 */
class BirchClustering8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit BirchClustering8(QObject* parent = nullptr);

    /** @brief 设置分支因子 */
    void setBranchFactor(int factor);

    /** @brief 设置聚类阈值 */
    void setThreshold(double threshold);

    /** @brief 对输入数据执行BIRCH聚类 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
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

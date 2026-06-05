#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类算法
 *
 * 基于CF树的大规模数据集聚类，适合内存受限场景。
 */
class BirchClustering7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsAbsorbed = 0;
        int totalSubclusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering7(QObject* parent = nullptr);

    /** @brief 构建CF树并执行聚类 */
    bool fit(const QVector<QVector<double>>& data, double threshold, int branchingFactor = 50);

    /** @brief 增量添加数据点到CF树 */
    void addPoint(const QVector<double>& point);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void subclusterFormed(int pointCount, double radius);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = 0.5;
};

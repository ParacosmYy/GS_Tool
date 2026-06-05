#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DBSCAN密度聚类算法实现(变体15)
 *
 * 基于密度的空间聚类方法，结合HDBSCAN的层次思想优化核心点选择策略，
 * 支持自适应邻域半径，能够处理不同密度的簇分布。
 */
class DBSCAN15 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit DBSCAN15(QObject* parent = nullptr);

    /** @brief 设置邻域半径参数epsilon */
    void setEpsilon(double epsilon);

    /** @brief 设置最小邻域点数MinPts */
    void setMinPts(int minPts);

    /** @brief 启用自适应邻域模式，根据局部密度自动调整半径 */
    void setAdaptiveEpsilon(bool enabled);

    /** @brief 对输入数据执行DBSCAN聚类，返回每个点的簇标签(-1为噪声) */
    QVector<int> fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号，返回簇数和噪声点数 */
    void clusteringCompleted(int clusterCount, int noiseCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_epsilon = 0.5;
    int m_minPts = 5;
    bool m_adaptive = false;
};

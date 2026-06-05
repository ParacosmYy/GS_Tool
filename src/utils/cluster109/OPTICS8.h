#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS聚类算法实现(Ordering Points To Identify the Clustering Structure)
 *
 * 生成数据点的可达距离排序，通过陡峭 valley 方法提取不同密度的簇，
 * 克服DBSCAN对单一参数的敏感性，适用于密度变化的多尺度聚类场景。
 */
class OPTICS8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit OPTICS8(QObject* parent = nullptr);

    /** @brief 设置核心距离邻域半径epsilon上限 */
    void setEpsilon(double epsilon);

    /** @brief 设置最小邻域点数MinPts，影响核心点判定 */
    void setMinPts(int minPts);

    /** @brief 对输入数据执行OPTICS排序，返回(点索引,可达距离)有序列表 */
    QVector<QPair<int, double>> fit(const QVector<QVector<double>>& data);

    /** @brief 从可达距离排序中提取指定聚类陡度阈值的簇标签 */
    QVector<int> extractClusters(double steepThreshold);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 排序完成信号，返回处理点数和可达距离范围 */
    void orderingCompleted(int pointCount, double maxReachability);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_epsilon = 1.0;
    int m_minPts = 5;
    QVector<QPair<int, double>> m_orderingCache;
};

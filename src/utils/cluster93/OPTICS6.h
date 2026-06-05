#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类算法实现
 *
 * 基于排序的点识别聚类结构,生成可达距离图以揭示多尺度聚类层次,
 * 相比DBSCAN能够发现不同密度的簇,适用于空间数据挖掘。
 */
class OPTICS6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit OPTICS6(QObject* parent = nullptr);

    /** @brief 设置邻域半径上限 */
    void setEpsilon(double epsilon);

    /** @brief 设置最小核心点数 */
    void setMinPts(int minPts);

    /** @brief 对输入数据执行OPTICS聚类分析 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 聚类完成信号,返回排序点数 */
    void clusteringCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_epsilon = 1.0;
    int m_minPts = 5;
};

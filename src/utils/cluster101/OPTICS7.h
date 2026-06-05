#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief OPTICS聚类算法实现
 *
 * 基于有序可达性的密度聚类方法，生成聚类有序序列，
 * 可通过可达距离图提取不同密度的簇结构。
 */
class OPTICS7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit OPTICS7(QObject* parent = nullptr);

    /** @brief 设置邻域半径上限epsilon */
    void setEpsilon(double epsilon);

    /** @brief 设置最小邻域点数MinPts */
    void setMinPts(int minPts);

    /** @brief 对输入数据执行OPTICS聚类 */
    QVector<QPair<int,double>> fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 */
    void clusteringCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_epsilon = 1.0;
    int m_minPts = 5;
};

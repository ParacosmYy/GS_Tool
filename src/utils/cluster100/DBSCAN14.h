#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DBSCAN密度聚类算法实现
 *
 * 基于密度的空间聚类方法，能够发现任意形状的簇并识别噪声点，
 * 无需预先指定簇数量。
 */
class DBSCAN14 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit DBSCAN14(QObject* parent = nullptr);

    /** @brief 设置邻域半径参数epsilon */
    void setEpsilon(double epsilon);

    /** @brief 设置最小邻域点数MinPts */
    void setMinPts(int minPts);

    /** @brief 对输入数据执行DBSCAN聚类 */
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
};

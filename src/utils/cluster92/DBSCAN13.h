#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类算法实现
 *
 * 基于密度的空间聚类算法,能够发现任意形状的簇并识别噪声点,
 * 无需预先指定簇数,适用于空间数据分析与异常检测。
 */
class DBSCAN13 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit DBSCAN13(QObject* parent = nullptr);

    /** @brief 设置邻域半径 */
    void setEpsilon(double epsilon);

    /** @brief 设置最小核心点邻居数 */
    void setMinPts(int minPts);

    /** @brief 对输入数据执行DBSCAN聚类 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 聚类完成信号,返回簇数与噪声点数 */
    void clusteringCompleted(int clusterCount, int noiseCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_epsilon = 0.5;
    int m_minPts = 5;
};

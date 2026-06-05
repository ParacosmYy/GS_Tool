#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief K-Means聚类算法实现 (11次迭代优化)
 *
 * 提供向量数据的K-Means聚类分析，支持多种距离度量和迭代优化。
 */
class KMeans11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalClusteringRuns = 0;   ///< 总聚类运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalIterations = 0;       ///< 总迭代次数
    };

    explicit KMeans11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行K-Means聚类
     * @param data 输入数据向量
     * @param k 聚类中心数量
     * @param maxIter 最大迭代次数
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data, int k, int maxIter = 100);

    /**
     * @brief 预测新样本所属簇
     * @param sample 输入样本
     * @return 簇标签索引
     */
    int predict(const QVector<double>& sample) const;

    /**
     * @brief 获取聚类中心
     * @return 各簇中心坐标
     */
    QVector<QVector<double>> centroids() const { return m_centroids; }

    /**
     * @brief 计算簇内误差平方和
     * @return SSE值
     */
    double computeSSE() const;

signals:
    /// 聚类完成信号
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;
};

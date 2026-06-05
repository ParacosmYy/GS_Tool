#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 谱聚类分析器
 *
 * 基于图拉普拉斯矩阵的特征分解进行聚类，
 * 适用于非凸簇形状的数据集。
 */
class SpectralClustering7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalPoints = 0;         ///< 已处理数据点数
        int totalClusters = 0;       ///< 聚类数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SpectralClustering7(QObject* parent = nullptr);

    /** @brief 设置目标聚类数 */
    void setClusterCount(int k);
    /** @brief 设置相似度核函数(rbf/poly/knn) */
    void setKernel(const QString& kernel);
    /** @brief 拟合数据，执行谱聚类 */
    void fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成，返回簇数 */
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_clusterCount = 3;
    QString m_kernel = "rbf";
};

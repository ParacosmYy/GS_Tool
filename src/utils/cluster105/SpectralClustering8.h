#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 谱聚类算法实现 (8维嵌入空间)
 *
 * 基于图拉普拉斯矩阵的特征分解进行聚类，适用于非凸簇结构。
 */
class SpectralClustering8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalClusteringRuns = 0;   ///< 总聚类运行次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int embeddingDimensions = 0;    ///< 嵌入维度
    };

    explicit SpectralClustering8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行谱聚类
     * @param similarityMatrix 相似度矩阵
     * @param k 目标簇数
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& similarityMatrix, int k);

    /**
     * @brief 从距离矩阵构建相似度矩阵
     * @param distanceMatrix 距离矩阵
     * @param sigma 高斯核宽度参数
     * @return 相似度矩阵
     */
    QVector<QVector<double>> buildSimilarityMatrix(const QVector<QVector<double>>& distanceMatrix, double sigma) const;

    /**
     * @brief 获取嵌入向量
     * @return 低维嵌入表示
     */
    QVector<QVector<double>> embeddings() const { return m_embeddings; }

    /**
     * @brief 设置归一化方式
     * @param normalized 是否使用归一化拉普拉斯
     */
    void setNormalized(bool normalized) { m_normalized = normalized; }

signals:
    /// 聚类完成信号
    void clusteringCompleted(int clusterCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    bool m_normalized = true;
    QVector<QVector<double>> m_embeddings;
};

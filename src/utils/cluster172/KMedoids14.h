/**
 * @file KMedoids14.h
 * @brief K-Medoids聚类(CLARA采样+轮廓系数验证) — K-Medoids Clustering with CLARA Sampling and Silhouette Validation
 *
 * 功能: 实现K-Medoids/PAM聚类算法，支持CLARA采样加速大规模数据集、
 *       轮廓系数(Silhouette)聚类质量验证。
 *
 * 协作: GaussianMixture12(高斯混合) / KMeans15(K-Means) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-Medoids聚类器(CLARA采样)
 */
class KMedoids14 : public QObject {
    Q_OBJECT

public:
    /** @brief 距离度量类型 */
    enum DistanceMetric { Euclidean = 0, Manhattan = 1 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int lastK = 0;                    ///< 最近使用的K值
        double lastSilhouette = 0.0;      ///< 最近轮廓系数
        double lastCost = 0.0;            ///< 最近总代价
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit KMedoids14(QObject *parent = nullptr);
    ~KMedoids14() override;

    void setK(int k);
    void setDistanceMetric(DistanceMetric metric);
    void setMaxIterations(int iter);
    void setClaraSamples(int samples);
    void setClaraSampleRatio(double ratio);

    /**
     * @brief 执行K-Medoids聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief CLARA采样聚类(适合大数据集) */
    QVector<int> fitClara(const QVector<QVector<double>>& data);

    /** @brief 计算轮廓系数验证聚类质量 */
    double silhouetteScore(const QVector<QVector<double>>& data,
                           const QVector<int>& labels) const;

    /** @brief 获取Medoid索引 */
    QVector<int> medoidIndices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, double cost, double silhouette);
    void claraIterationCompleted(int iter, double bestCost);

private:
    /** @brief 计算两点间距离 */
    double distance(const QVector<double>& a,
                    const QVector<double>& b) const;

    /** @brief PAM BUILD阶段: 初始化medoids */
    void pamBuild(const QVector<QVector<double>>& data,
                  const QVector<int>& indices);

    /** @brief PAM SWAP阶段: 交换优化medoids */
    double pamSwap(const QVector<QVector<double>>& data,
                   const QVector<int>& indices);

    /** @brief 分配样本到最近medoid */
    QVector<int> assignClusters(const QVector<QVector<double>>& data,
                                const QVector<int>& indices) const;

    /** @brief 计算单个样本的轮廓系数 */
    double singleSilhouette(const QVector<QVector<double>>& data,
                            const QVector<int>& labels, int i) const;

    int m_k = 3;
    DistanceMetric m_metric = Euclidean;
    int m_maxIter = 100;
    int m_claraSamples = 5;
    double m_claraRatio = 0.25;

    QVector<int> m_medoidIdx;  ///< medoid在data中的索引
    QVector<int> m_labels;
    QVector<double> m_distCache; ///< 预计算距离矩阵(上三角)

    Stats m_stats;
    double m_timeSum = 0.0;
};

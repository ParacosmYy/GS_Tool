/**
 * @file KMeans16.h
 * @brief K均值聚类(k-means++初始化+小批量变体+二分K层次优化) — K-means Clustering with k-means++ Seeding, Mini-batch Variant and Bisecting-K Hierarchical Refinement
 *
 * 功能: 实现K均值聚类算法，支持k-means++智能初始化、小批量(mini-batch)加速、
 *       二分K均值(Bisecting-K)层次优化和轮廓系数评估。
 *
 * 协作: SpectralCluster7(谱聚类) / FuzzyCMeans7(模糊C均值) / KMedoids14(K-Medoids)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K均值聚类器(k-means++/mini-batch/bisecting-K)
 */
class KMeans16 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numClusters = 0;
        int numSamples = 0;
        int numIterations = 0;
        double inertia = 0.0;
        double silhouette = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 聚类模式 */
    enum Mode {
        Standard,     ///< Classic Lloyd's algorithm
        MiniBatch,    ///< Mini-batch stochastic
        BisectingK    ///< Bisecting K-means hierarchical
    };

    explicit KMeans16(QObject *parent = nullptr);
    ~KMeans16() override;

    void setNumClusters(int k);
    void setMaxIterations(int iter);
    void setMode(Mode mode);
    void setBatchSize(int size);
    void setTolerance(double tol);
    void setSeed(int seed);

    /** @brief 执行聚类，返回样本标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 预测新样本的簇标签 */
    int predict(const QVector<double>& sample) const;

    /** @brief k-means++初始化 */
    QVector<QVector<double>> kmeansPPInit(const QVector<QVector<double>>& data, int k);

    /** @brief 轮廓系数评估 */
    double silhouetteScore(const QVector<QVector<double>>& data,
                           const QVector<int>& labels) const;

    /** @brief 获取聚类中心 */
    QVector<QVector<double>> centers() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, double inertia, double silhouette);

private:
    int m_numClusters = 3;
    int m_maxIter = 300;
    Mode m_mode = Standard;
    int m_batchSize = 32;
    double m_tol = 1e-4;
    int m_seed = 42;

    int m_n = 0;
    int m_dims = 0;
    QVector<int> m_labels;
    QVector<QVector<double>> m_centers;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 欧氏距离平方 */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief 标准Lloyd迭代 */
    QVector<int> lloydsIterate(const QVector<QVector<double>>& data);

    /** @brief 小批量K均值 */
    QVector<int> miniBatchFit(const QVector<QVector<double>>& data);

    /** @brief 二分K均值 */
    QVector<int> bisectingFit(const QVector<QVector<double>>& data);

    /** @brief 单次K均值(给定初始中心) */
    QVector<int> singleKMeans(const QVector<QVector<double>>& data,
                               QVector<QVector<double>>& initCenters);

    /** @brief 计算簇内惯性 */
    double computeInertia(const QVector<QVector<double>>& data) const;
};

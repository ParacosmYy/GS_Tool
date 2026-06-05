/**
 * @file KMedoids13.h
 * @brief K-Medoids聚类算法(PAM) — K-Medoids Clustering with PAM
 *
 * 功能: 支持随机/启发式/K-Medoids++三种初始化策略，PAM(BUILD+SWAP)
 *       迭代优化，轮廓系数评估。适用于小到中等规模数据的鲁棒聚类。
 *
 * 协作: KMeans5(K-Means变体) / HierarchicalDensity(层次密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief K-Medoids聚类器，基于PAM(Partitioning Around Medoids)算法
 */
class KMedoids13 : public QObject {
    Q_OBJECT

public:
    /** @brief 初始化策略 */
    enum class InitMethod {
        Random,         ///< 随机选择K个medoid
        Heuristic,      ///< 启发式：选最集中点
        KMedoidsPP      ///< K-Medoids++：距离加权概率
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalClusterOps = 0;       ///< 累计聚类操作次数
        quint64 totalIterations = 0;       ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        double silhouetteScore = 0.0;      ///< 最近一次的轮廓系数
    };

    explicit KMedoids13(QObject* parent = nullptr);

    /**
     * @brief 设置簇数量K
     * @param k 簇的数量，必须 >= 2
     */
    void setK(int k);

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 设置初始化方法
     * @param method 初始化策略
     */
    void setInitMethod(InitMethod method);

    /**
     * @brief 执行K-Medoids聚类(PAM)
     * @param data 输入数据，每个元素为一个特征向量
     * @return 每个样本的簇标签(0到K-1)
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /**
     * @brief 预测新样本的簇归属
     * @param data 待预测样本
     * @return 簇标签
     */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief 获取当前medoid索引 */
    QVector<int> medoidIndices() const { return m_medoidIndices; }

    /** @brief 获取当前medoid向量 */
    QVector<QVector<double>> medoids() const { return m_medoids; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param k 簇数 @param iterations 实际迭代次数 */
    void fitCompleted(int k, int iterations, double silhouette);

private:
    /** @brief 距离矩阵预计算 */
    QVector<QVector<double>> computeDistanceMatrix(const QVector<QVector<double>>& data) const;

    /** @brief 随机初始化 */
    void initRandom(int n);
    /** @brief 启发式初始化 */
    void initHeuristic(const QVector<QVector<double>>& distMatrix);
    /** @brief K-Medoids++初始化 */
    void initKMedoidsPP(const QVector<QVector<double>>& distMatrix);

    /** @brief 欧氏距离 */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief 计算轮廓系数 */
    double computeSilhouette(const QVector<int>& labels,
                             const QVector<QVector<double>>& distMatrix) const;

    int m_k = 3;
    int m_maxIter = 100;
    InitMethod m_initMethod = InitMethod::KMedoidsPP;

    QVector<int> m_medoidIndices;
    QVector<QVector<double>> m_medoids;
    QVector<QVector<double>> m_lastData;

    Stats m_stats;
    double m_timeSum = 0.0;
};

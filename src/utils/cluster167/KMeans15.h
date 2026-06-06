/**
 * @file KMeans15.h
 * @brief K-Means++(k-means||初始化+小批量变体) — K-Means++ with k-means|| Initialization and Mini-Batch Variant
 *
 * 功能: 实现K-Means++聚类，支持k-means||并行初始化、Lloyd迭代和小批量变体，
 *       适用于大规模数据集的快速聚类与嵌入式信号分箱。
 *
 * 协作: SpectralCluster6(谱聚类) / GaussianMixture11(GMM) / DBSCAN5(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief K-Means++聚类器
 */
class KMeans15 : public QObject {
    Q_OBJECT

public:
    /** @brief 初始化方法 */
    enum InitMethod { KMeansPP, KMeansParallel, Random };

    /** @brief 运行模式 */
    enum RunMode { Lloyd, MiniBatch };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        int lastK = 0;                   ///< 最近k值
        int lastIterations = 0;          ///< 最近迭代数
        double lastInertia = 0.0;        ///< 最近惯性值
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit KMeans15(QObject *parent = nullptr);
    ~KMeans15() override;

    void setInitMethod(InitMethod method);
    void setRunMode(RunMode mode);
    void setMaxIterations(int maxIter);
    void setMiniBatchSize(int batchSize);
    void setTolerance(double tol);

    /**
     * @brief 执行聚类
     * @param data 数据集(每行一个样本)
     * @param k 簇数
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data, int k);

    /** @brief 预测新样本的簇标签 */
    int predict(const QVector<double>& sample) const;

    /** @brief 获取聚类中心 */
    QVector<QVector<double>> centroids() const;

    /** @brief 计算惯性(样本到最近中心距离之和) */
    double inertia(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param k 簇数 @param iters 迭代数 */
    void clusteringCompleted(int k, int iters);

private:
    /** @brief K-Means++初始化 */
    void initKMeansPP(const QVector<QVector<double>>& data, int k);

    /** @brief k-means||并行初始化 */
    void initKMeansParallel(const QVector<QVector<double>>& data, int k);

    /** @brief 随机初始化 */
    void initRandom(const QVector<QVector<double>>& data, int k);

    /** @brief Lloyd迭代 */
    QVector<int> runLloyd(const QVector<QVector<double>>& data, int k);

    /** @brief 小批量迭代 */
    QVector<int> runMiniBatch(const QVector<QVector<double>>& data, int k);

    /** @brief 分配样本到最近中心 */
    int assignCluster(const QVector<double>& sample) const;

    /** @brief 欧氏距离平方 */
    static double distSq(const QVector<double>& a, const QVector<double>& b);

    InitMethod m_init = KMeansPP;
    RunMode m_mode = Lloyd;
    int m_maxIter = 300;
    int m_batchSize = 100;
    double m_tol = 1e-6;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;
};

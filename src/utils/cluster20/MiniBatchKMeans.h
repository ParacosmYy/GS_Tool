/**
 * @file MiniBatchKMeans.h
 * @brief 小批量K均值聚类 — 随机梯度下降优化聚类中心
 *
 * 功能: Mini-batch K-means 聚类算法，支持批量分配、收敛追踪、
 *       在线学习模式，适用于流式数据的增量聚类。
 *
 * 协作: DataClassifier(分类) / AnomalyDetector(异常聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 小批量K均值聚类引擎
 *
 * 通过随机小批量样本迭代更新聚类中心，相比标准K-means
 * 在大规模数据上收敛更快，内存占用更小。
 */
class MiniBatchKMeans : public QObject {
    Q_OBJECT

public:
    /** @brief 初始化模式 */
    enum class InitMethod {
        Random,         ///< 随机选取初始中心
        KMeansPlusPlus  ///< K-means++ 距离加权选取
    };
    Q_ENUM(InitMethod)

    /** @brief 聚类结果 */
    struct ClusterResult {
        int clusterId = -1;             ///< 聚类ID
        double distance = 0.0;          ///< 到中心距离
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalBatches = 0;           ///< 累计批次数
        quint64 totalPointsAssigned = 0;    ///< 累计分配点数
        double  totalInertia = 0.0;         ///< 累计惯性(误差平方和)
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        int     iterationsRun = 0;          ///< 已运行迭代次数
    };

    explicit MiniBatchKMeans(QObject* parent = nullptr);

    void setClusterCount(int k);
    void setBatchSize(int size);
    void setMaxIterations(int iterations);
    void setInitMethod(InitMethod method);

    void fit(const QVector<QVector<double>>& data);
    void partialFit(const QVector<QVector<double>>& batch);
    QList<ClusterResult> predict(const QVector<QVector<double>>& data) const;
    QVector<QVector<double>> centroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iteration, double inertia);
    void fittingFinished(int iterations, double finalInertia);

private:
    void initializeCentroids(const QVector<QVector<double>>& data);
    void updateCentroids(const QVector<QVector<double>>& batch);
    double computeInertia(const QVector<QVector<double>>& data) const;
    int findNearestCluster(const QVector<double>& point) const;
    double euclideanDistance(const QVector<double>& a,
                            const QVector<double>& b) const;

    int m_k;                        ///< 聚类数
    int m_batchSize;                ///< 批量大小
    int m_maxIterations;            ///< 最大迭代次数
    InitMethod m_initMethod;        ///< 初始化方法

    QVector<QVector<double>> m_centroids;   ///< 聚类中心
    QVector<int> m_counts;                  ///< 各中心累积计数

    Stats m_stats;
    double m_timeSum = 0.0;
};

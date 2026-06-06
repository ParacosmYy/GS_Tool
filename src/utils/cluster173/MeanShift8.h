/**
 * @file MeanShift8.h
 * @brief 均值漂移聚类(变带宽气球估计器+模式收敛) — Mean Shift Clustering with Variable Bandwidth Balloon Estimator and Mode Convergence
 *
 * 功能: 实现Mean Shift聚类算法，支持气球估计器变带宽核密度估计、
 *       自适应模式收敛检测和簇合并。
 *
 * 协作: KMedoids14(K-Medoids) / KMeans15(K-Means) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 均值漂移聚类器(变带宽气球估计器)
 */
class MeanShift8 : public QObject {
    Q_OBJECT

public:
    /** @brief 核函数类型 */
    enum Kernel { Gaussian = 0, Epanechnikov = 1, Uniform = 2 };

    /** @brief 带宽策略 */
    enum BandwidthStrategy { Fixed = 0, Balloon = 1, KNN = 2 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int numClusters = 0;              ///< 最近聚类数
        int numIterations = 0;            ///< 最近总迭代次数
        double avgBandwidth = 0.0;        ///< 平均带宽
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit MeanShift8(QObject *parent = nullptr);
    ~MeanShift8() override;

    void setKernel(Kernel k);
    void setBandwidthStrategy(BandwidthStrategy strategy);
    void setBandwidth(double h);
    void setKNNK(int k);
    void setMaxIterations(int iter);
    void setConvergenceThreshold(double eps);
    void setClusterMergeThreshold(double t);

    /**
     * @brief 执行均值漂移聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取模式点(簇中心) */
    QVector<QVector<double>> modes() const;

    /** @brief 获取每个点的带宽(气球估计) */
    QVector<double> bandwidths() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int iterations);
    void iterationProgress(int current, int total);

private:
    /** @brief 计算核函数值 */
    double kernelValue(double dist, double h) const;

    /** @brief 计算单点的气球带宽 */
    double balloonBandwidth(const QVector<QVector<double>>& data,
                            int idx) const;

    /** @brief 计算KNN带宽 */
    double knnBandwidth(const QVector<QVector<double>>& data,
                        int idx) const;

    /** @brief 对单个点执行均值漂移直到收敛 */
    QVector<double> shiftPoint(const QVector<QVector<double>>& data,
                               const QVector<double>& point,
                               const QVector<double>& bw) const;

    /** @brief 合并相近模式点 */
    void mergeModes(const QVector<double>& mergeDist);

    /** @brief 欧氏距离 */
    static double euclidean(const QVector<double>& a,
                            const QVector<double>& b);

    Kernel m_kernel = Gaussian;
    BandwidthStrategy m_bwStrategy = Fixed;
    double m_bandwidth = 1.0;
    int m_knnK = 5;
    int m_maxIter = 300;
    double m_convergenceEps = 1e-4;
    double m_mergeThreshold = 0.1;

    QVector<QVector<double>> m_modes;  ///< 模式点(簇中心)
    QVector<double> m_bandwidths;      ///< 每点带宽
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;
};

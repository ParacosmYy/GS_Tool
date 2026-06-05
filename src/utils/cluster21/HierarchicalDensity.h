/**
 * @file HierarchicalDensity.h
 * @brief 层次密度聚类 — 密度峰值发现/决策图/聚类中心选择/Halo分配
 *
 * 功能: 基于Rodriguez-Laio密度峰值聚类算法，自动发现聚类中心，
 *       支持局部密度计算、决策图绘制、Halo噪声点标记。
 *
 * 协作: PeakDetector(峰值检测) / DataClassifier(数据分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 层次密度聚类引擎
 */
class HierarchicalDensity : public QObject {
    Q_OBJECT

public:
    /** @brief 密度计算方法 */
    enum class DensityMethod {
        GaussianKernel,  ///< 高斯核密度
        CutoffRadius,    ///< 截断半径计数
        KNN              ///< K近邻距离倒数
    };
    Q_ENUM(DensityMethod)

    /** @brief 聚类点信息 */
    struct ClusterPoint {
        int index = 0;              ///< 原始数据索引
        double density = 0.0;       ///< 局部密度
        double delta = 0.0;         ///< 最小距离(到更高密度点)
        int nearestHigher = -1;     ///< 最近更高密度点索引
        int clusterId = -1;         ///< 聚类编号(-1为噪声)
        bool isCenter = false;      ///< 是否为聚类中心
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalClusterOps = 0;       ///< 累计聚类操作次数
        quint64 totalPointsProcessed = 0;  ///< 累计处理数据点数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        int     maxClusters = 0;           ///< 历史最大聚类数
    };

    explicit HierarchicalDensity(QObject* parent = nullptr);

    /** @brief 设置密度计算方法 @param method 方法 */
    void setDensityMethod(DensityMethod method);

    /** @brief 设置截断半径(百分比) @param ratio 比例(0-1) */
    void setCutoffRatio(double ratio);

    /** @brief 设置高斯核带宽 @param sigma 带宽参数 */
    void setGaussianSigma(double sigma);

    /** @brief 执行聚类 @param data 二维数据(每行一个点) @return 聚类结果 */
    QList<ClusterPoint> cluster(const QVector<QVector<double>>& data);

    /** @brief 获取决策图数据 @return (密度数组, delta数组) */
    QPair<QVector<double>, QVector<double>> decisionGraph() const;

    /** @brief 获取指定聚类的点索引 @param clusterId 聚类ID @return 点索引列表 */
    QList<int> clusterMembers(int clusterId) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param numClusters 聚类数 @param numPoints 总点数 */
    void clusteringComplete(int numClusters, int numPoints);

    /** @brief 聚类中心已选择 @param centers 中心索引列表 */
    void centersSelected(const QList<int>& centers);

private:
    void computePairwiseDistances(const QVector<QVector<double>>& data);
    QVector<double> computeDensity(int n);
    QVector<double> computeDelta(int n, const QVector<double>& density);
    QList<int> selectCenters(int n, const QVector<double>& density,
                             const QVector<double>& delta);
    void assignClusters(int n, const QList<int>& centers,
                        const QVector<double>& density);
    void assignHalo(int n);

    DensityMethod m_method;         ///< 密度方法
    double m_cutoffRatio;           ///< 截断半径比例
    double m_sigma;                 ///< 高斯核带宽
    double m_dc;                    ///< 计算得到的截断距离

    QVector<QVector<double>> m_distMatrix; ///< 距离矩阵
    QList<ClusterPoint> m_results;         ///< 最新聚类结果

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

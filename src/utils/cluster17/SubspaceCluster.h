/**
 * @file SubspaceCluster.h
 * @brief 子空间聚类引擎 — PROCLUS算法/维度选择/聚类质量评估
 *
 * 功能: 在高维数据中发现轴平行的子空间聚类，支持PROCLUS投影
 *       聚类、维度相关性评估、聚类质量度量(轮廓系数/SSE)。
 *
 * 协作: DataQualityScorer(数据质量) / SpectrumAnalyzer(频谱特征聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 子空间聚类引擎 — 高维数据子空间发现
 */
class SubspaceCluster : public QObject {
    Q_OBJECT

public:
    /** @brief 单个聚类结果 */
    struct Cluster {
        QVector<int> pointIndices;      ///< 聚类包含的数据点索引
        QVector<int> selectedDims;      ///< 选中的子空间维度
        double quality = 0.0;           ///< 聚类质量分数
        QVector<double> centroid;       ///< 聚类中心坐标
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalClusterOps = 0;        ///< 累计聚类操作次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理数据点数
        quint64 totalDimensionsAnalyzed = 0;///< 累计分析维度数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  bestQuality = 0.0;          ///< 历史最佳聚类质量
    };

    explicit SubspaceCluster(QObject* parent = nullptr);

    /**
     * @brief 执行PROCLUS子空间聚类
     * @param data 数据矩阵 [nPoints x nDims]
     * @param k 目标聚类数
     * @param l 平均子空间维度数
     * @return 聚类结果列表
     */
    QList<Cluster> proclus(const QVector<QVector<double>>& data,
                           int k, int l);

    /**
     * @brief 为指定数据点选择最佳子空间维度
     * @param data 数据矩阵
     * @param medoidIdx 中心点索引
     * @param l 目标维度数
     * @return 选中的维度索引列表
     */
    QVector<int> selectDimensions(const QVector<QVector<double>>& data,
                                  int medoidIdx, int l);

    /**
     * @brief 计算轮廓系数(聚类质量)
     * @param data 数据矩阵
     * @param clusters 聚类结果
     * @return 平均轮廓系数 [-1, 1]
     */
    double silhouetteScore(const QVector<QVector<double>>& data,
                           const QList<Cluster>& clusters);

    /**
     * @brief 计算轴平行子空间投影
     * @param data 数据矩阵
     * @param dims 投影维度索引
     * @return 投影后的数据
     */
    QVector<QVector<double>> axisParallelProject(
        const QVector<QVector<double>>& data,
        const QVector<int>& dims);

    /**
     * @brief 计算聚类SSE(组内平方误差)
     * @param data 数据矩阵
     * @param cluster 聚类
     * @return SSE值
     */
    double clusterSSE(const QVector<QVector<double>>& data,
                      const Cluster& cluster) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成 @param numClusters 聚类数 @param quality 整体质量 */
    void clusteringComplete(int numClusters, double quality);

    /** @brief 维度选择完成 @param dims 选中的维度 @param score 维度得分 */
    void dimensionsSelected(const QVector<int>& dims, double score);

private:
    double manhattanDist(const QVector<double>& a,
                         const QVector<double>& b) const;
    QVector<int> findMedoids(const QVector<QVector<double>>& data, int k);
    double computeDimVar(const QVector<QVector<double>>& data,
                         int dim, int medoidIdx);

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};

/**
 * @file SpectralCluster2.h
 * @brief 谱聚类增强 — 归一化割/随机游走/特征向量选择/k-means后处理
 *
 * 基于图拉普拉斯矩阵的谱分解进行聚类，支持三种归一化方式:
 *   - 归一化割(Normalized Cut)
 *   - 随机游走(Random Walk)
 *   - 未归一化(Unnormalized)
 * 通过特征向量选择策略自动确定聚类数，使用k-means完成最终分配。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QPair>

/**
 * @brief 谱聚类增强算法
 */
class SpectralCluster2 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalClusterOps = 0;      ///< 总聚类操作次数
        quint64 totalEigenDecomps = 0;    ///< 总特征分解次数
        quint64 totalKmeansIters = 0;     ///< 总k-means迭代次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 归一化模式 */
    enum class NormalizeMode {
        NormalizedCut,  ///< 归一化割: L_sym = D^{-1/2} L D^{-1/2}
        RandomWalk,     ///< 随机游走: L_rw = D^{-1} L
        Unnormalized    ///< 未归一化: L = D - W
    };
    Q_ENUM(NormalizeMode)

    /**
     * @brief 构造函数
     * @param k 聚类数(0=自动检测)
     * @param mode 归一化模式
     * @param parent 父对象
     */
    explicit SpectralCluster2(int k = 0, NormalizeMode mode = NormalizeMode::NormalizedCut,
                              QObject* parent = nullptr);

    /**
     * @brief 从相似度矩阵执行聚类
     * @param similarity NxN相似度矩阵(对称, 行优先)
     * @param n 数据点数量
     * @return 每个点的聚类标签(0~k-1)
     */
    QVector<int> fit(const QVector<double>& similarity, int n);

    /**
     * @brief 从坐标点构建相似度矩阵并聚类
     * @param points 坐标点列表 [x0,y0,x1,y1,...]
     * @param sigma RBF核带宽(<=0则自动估计)
     * @return 每个点的聚类标签
     */
    QVector<int> fitPoints(const QVector<double>& points, double sigma = 0.0);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 获取最近一次的特征值 */
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    /** @brief 获取聚类数 */
    int clusterCount() const { return m_k; }
    /** @brief 设置聚类数 */
    void setClusterCount(int k) { m_k = k; }
    /** @brief 设置归一化模式 */
    void setNormalizeMode(NormalizeMode mode) { m_mode = mode; }

signals:
    /** 聚类完成 */
    void clusteringComplete(int k, double processingTimeMs);
    /** 特征分解完成 */
    void eigenDecompositionComplete(int nEigenvalues);

private:
    /** 构建度矩阵对角线 */
    QVector<double> buildDegree(const QVector<double>& W, int n) const;
    /** 构建归一化拉普拉斯 */
    QVector<double> buildLaplacian(const QVector<double>& W, int n,
                                   NormalizeMode mode) const;
    /** 幂迭代法求前k个特征向量 */
    void eigenDecompose(const QVector<double>& mat, int n, int k);
    /** 自动选择聚类数(特征间隙法) */
    int autoSelectK(const QVector<double>& evals) const;
    /** k-means后处理 */
    QVector<int> kmeansPostProcess(const QVector<double>& vectors, int n, int dim, int k);

    int m_k;
    NormalizeMode m_mode;
    QVector<double> m_eigenvalues;
    QVector<double> m_eigenvectors;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};

/**
 * @file GaussianMixture.h
 * @brief 高斯混合模型(GMM) — EM算法聚类与概率密度估计
 *
 * 功能: 基于期望最大化(EM)算法拟合多高斯分量混合模型，
 *       支持聚类预测、后验概率计算、模型参数查询。
 *       适用于信号分簇、异常检测、数据分布建模。
 *
 * 协作: StatDistribution(分布检验) / AnomalyDetector(异常分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 高斯混合模型 — EM算法实现
 */
class GaussianMixture : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;          ///< 累计拟合次数
        quint64 totalPredictions = 0;   ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param numComponents 高斯分量数(默认3)
     * @param maxIterations EM最大迭代次数(默认100)
     * @param parent 父对象
     */
    explicit GaussianMixture(int numComponents = 3,
                             int maxIterations = 100,
                             QObject* parent = nullptr);

    /**
     * @brief 拟合模型 — 使用EM算法估计参数
     * @param data 样本数据，每个样本为多维向量
     * @param dimensions 数据维度
     * @return 是否拟合成功
     */
    bool fit(const QVector<QVector<double>>& data, int dimensions);

    /**
     * @brief 预测聚类标签 — 返回最大后验概率对应的分量
     * @param sample 输入样本
     * @return 聚类标签(分量索引)
     */
    int predict(const QVector<double>& sample) const;

    /**
     * @brief 预测各分量后验概率
     * @param sample 输入样本
     * @return 各分量的后验概率向量
     */
    QVector<double> predictProbability(const QVector<double>& sample) const;

    /** @brief 获取各分量均值 @return 均值向量列表 */
    QVector<QVector<double>> means() const;

    /** @brief 获取各分量协方差矩阵 @return 协方差矩阵列表 */
    QVector<QVector<QVector<double>>> covariances() const;

    /** @brief 获取各分量混合权重 @return 权重向量 */
    QVector<double> weights() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 @param iterations 实际迭代次数 @param logLikelihood 最终对数似然 */
    void fitCompleted(int iterations, double logLikelihood);

    /** @brief 预测完成信号 @param label 聚类标签 @param confidence 置信度 */
    void predicted(int label, double confidence);

private:
    /** @brief 初始化参数(K-Means++) */
    void initializeParameters(const QVector<QVector<double>>& data);

    /** @brief E步 — 计算每个样本属于各分量的后验概率 */
    void expectationStep(const QVector<QVector<double>>& data,
                         QVector<QVector<double>>& responsibilities);

    /** @brief M步 — 根据后验概率更新模型参数 */
    void maximizationStep(const QVector<QVector<double>>& data,
                          const QVector<QVector<double>>& responsibilities);

    /** @brief 计算对数似然 */
    double computeLogLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief 计算多元高斯概率密度 */
    double gaussianPdf(const QVector<double>& x, int component) const;

    /** @brief 计算向量点积 */
    static double dotProduct(const QVector<double>& a,
                             const QVector<double>& b);

    /** @brief 解线性方程组(Cholesky) */
    static QVector<double> solveTriangular(
        const QVector<QVector<double>>& L,
        const QVector<double>& b);

    int m_numComponents;        ///< 高斯分量数
    int m_maxIterations;        ///< 最大迭代次数
    int m_dimensions;           ///< 数据维度
    double m_tolerance;         ///< 收敛阈值

    QVector<QVector<double>> m_means;           ///< 各分量均值
    QVector<QVector<QVector<double>>> m_covs;    ///< 各分量协方差矩阵
    QVector<QVector<QVector<double>>> m_covL;    ///< 协方差Cholesky下三角
    QVector<double> m_weights;                   ///< 混合权重
    QVector<double> m_logDet;                    ///< 各分量协方差行列式对数
    bool m_fitted;                               ///< 是否已拟合

    QElapsedTimer m_timer;    ///< 计时器
    double m_timeSum;          ///< 累计耗时
    Stats m_stats;
};

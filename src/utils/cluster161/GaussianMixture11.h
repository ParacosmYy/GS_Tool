/**
 * @file GaussianMixture11.h
 * @brief 高斯混合模型(对角协方差+BIC选择) — Gaussian Mixture Model with Diagonal Covariance and BIC
 *
 * 功能: 基于EM算法的高斯混合模型聚类，支持对角协方差矩阵估计。
 *       使用BIC(贝叶斯信息准则)自动选择最优分量数。
 *       提供软聚类概率、对数似然和分量参数查询。
 *
 * 协作: MeanShift7(均值漂移) / KMedoids13(PAM聚类) / DBSCAN8(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型聚类器
 */
class GaussianMixture11 : public QObject {
    Q_OBJECT

public:
    /** @brief 单个高斯分量参数 */
    struct Component {
        QVector<double> mean;           ///< 均值向量
        QVector<double> variance;       ///< 对角方差向量
        double weight;                  ///< 混合权重
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFits = 0;              ///< 累计拟合次数
        quint64 bestK = 0;                  ///< 最优分量数
        double bestBIC = 0.0;               ///< 最优BIC值
        double finalLogLikelihood = 0.0;    ///< 最终对数似然
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit GaussianMixture11(QObject* parent = nullptr);
    ~GaussianMixture11() override;

    /**
     * @brief 设置候选分量数范围
     * @param minK 最小分量数, >= 1
     * @param maxK 最大分量数
     */
    void setKRange(int minK, int maxK);

    /**
     * @brief 设置EM最大迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 设置EM收敛阈值(对数似然变化)
     * @param threshold 收敛阈值
     */
    void setConvergenceThreshold(double threshold);

    /**
     * @brief 拟合GMM并自动选择最优K(BIC)
     * @param data 输入数据,每个元素为特征向量
     * @return 每个样本的硬聚类标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /**
     * @brief 获取各分量参数
     */
    QVector<Component> components() const { return m_components; }

    /**
     * @brief 获取软聚类后验概率(N x K)
     */
    QVector<QVector<double>> responsibilities() const { return m_resp; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param k 最优分量数 @param bic BIC值 */
    void fitCompleted(int k, double bic);

private:
    /** @brief 用KMeans++初始化分量参数 */
    void initializeComponents(const QVector<QVector<double>>& data, int k);

    /** @brief E步：计算后验概率 */
    double expectationStep(const QVector<QVector<double>>& data);

    /** @brief M步：更新分量参数 */
    void maximizationStep(const QVector<QVector<double>>& data);

    /** @brief 计算单个点在指定分量下的对数概率密度 */
    double logGaussianPDF(const QVector<double>& x, int compIdx) const;

    /** @brief 计算BIC */
    double computeBIC(const QVector<QVector<double>>& data, double logLikelihood, int k) const;

    /** @brief 用指定K运行EM */
    double runEM(const QVector<QVector<double>>& data, int k);

    int m_minK = 1;
    int m_maxK = 8;
    int m_maxIterations = 100;
    double m_convergenceThreshold = 1e-6;

    QVector<Component> m_components;     ///< 当前分量参数
    QVector<QVector<double>> m_resp;     ///< 后验概率矩阵(N x K)

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file GaussianMixture2.h
 * @brief 一维高斯混合模型(EM算法) — 适合实时数据流
 *
 * 功能: 使用期望最大化(EM)算法对一维数据进行高斯混合建模，
 *       支持聚类预测、概率计算和参数查询，适用于实时数据流分析。
 *
 * 协作: StatDistribution(统计分布) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 一维高斯混合模型
 */
class GaussianMixture2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;              ///< 累计拟合次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param numComponents 混合成分数(默认3)
     * @param maxIterations 最大迭代次数(默认50)
     * @param parent 父对象
     */
    explicit GaussianMixture2(int numComponents = 3,
                              int maxIterations = 50,
                              QObject* parent = nullptr);

    /**
     * @brief 拟合数据
     * @param data 一维数据
     * @return 是否拟合成功
     */
    bool fit(const QVector<double>& data);

    /**
     * @brief 预测样本所属成分
     * @param sample 样本值
     * @return 成分索引(0-based)
     */
    int predict(double sample);

    /**
     * @brief 样本属于某成分的概率
     * @param sample 样本值
     * @param component 成分索引
     * @return 后验概率
     */
    double probability(double sample, int component);

    /**
     * @brief 获取各成分均值
     * @return 均值向量
     */
    QVector<double> means() const;

    /**
     * @brief 获取各成分方差
     * @return 方差向量
     */
    QVector<double> variances() const;

    /**
     * @brief 获取各成分混合权重
     * @return 权重向量
     */
    QVector<double> weights() const;

    /** @brief 是否已拟合 @return 拟合状态 */
    bool isFitted() const { return m_fitted; }

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param iterations 实际迭代数 @param converged 是否收敛 */
    void fitCompleted(int iterations, bool converged);

private:
    /**
     * @brief 计算高斯概率密度
     * @param x 输入值
     * @param mean 均值
     * @param variance 方差
     * @return 概率密度
     */
    double gaussianPdf(double x, double mean, double variance) const;

    /**
     * @brief 初始化参数(K-means++简化版)
     * @param data 输入数据
     */
    void initializeParameters(const QVector<double>& data);

    int m_numComponents;        ///< 成分数
    int m_maxIterations;        ///< 最大迭代数
    bool m_fitted;              ///< 拟合标志

    QVector<double> m_means;    ///< 各成分均值
    QVector<double> m_variances;///< 各成分方差
    QVector<double> m_weights;  ///< 各成分混合权重

    Stats m_stats;              ///< 统计信息
    double m_timeSum;           ///< 累计耗时
};

/**
 * @file GaussianMixture3.h
 * @brief 高斯混合模型 — EM算法/BIC/AIC模型选择
 *
 * 实现基于期望最大化(EM)算法的高斯混合模型(GMM):
 *   - 支持一维和多维数据
 *   - 自动或手动指定高斯分量数
 *   - BIC/AIC准则自动选择最佳分量数
 *   - 提供聚类预测和概率密度估计
 *
 * 协作: StatDistribution(分布拟合) / DataClassifier(分类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @class GaussianMixture3
 * @brief 高斯混合模型引擎
 *
 * 使用EM算法拟合数据的高斯混合分布，支持模型选择和聚类。
 * 适用于数据分类、异常检测和密度估计等场景。
 */
class GaussianMixture3 : public QObject
{
    Q_OBJECT

public:
    /** @brief 单个高斯分量 */
    struct GaussianComponent {
        double weight = 0.0;          ///< 混合权重
        double mean = 0.0;            ///< 均值
        double variance = 1.0;        ///< 方差
        QVector<double> responsibilities; ///< 后验概率(E步)
    };

    /** @brief 拟合结果 */
    struct FitResult {
        QVector<GaussianComponent> components; ///< 高斯分量列表
        double logLikelihood = 0.0;            ///< 对数似然
        double bic = 0.0;                      ///< BIC值
        double aic = 0.0;                      ///< AIC值
        int iterations = 0;                    ///< EM迭代次数
        bool converged = false;                ///< 是否收敛
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;                ///< 累计拟合次数
        quint64 totalPredictions = 0;         ///< 累计预测次数
        quint64 totalSamplesProcessed = 0;    ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0;    ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit GaussianMixture3(QObject* parent = nullptr);

    /**
     * @brief 设置高斯分量数
     * @param k 分量数(1-10)
     */
    void setComponentCount(int k);

    /**
     * @brief 设置EM算法参数
     * @param maxIterations 最大迭代次数
     * @param tolerance 收敛阈值
     */
    void setEmParameters(int maxIterations, double tolerance);

    /**
     * @brief 拟合数据
     * @param data 一维数据样本
     * @return 拟合结果
     */
    FitResult fit(const QVector<double>& data);

    /**
     * @brief 自动选择最佳分量数并拟合(BIC准则)
     * @param data 数据样本
     * @param maxK 最大候选分量数(默认5)
     * @return 最佳拟合结果
     */
    FitResult fitAuto(const QVector<double>& data, int maxK = 5);

    /**
     * @brief 预测样本所属聚类
     * @param sample 输入样本
     * @return 聚类ID(0-based)
     */
    int predict(double sample) const;

    /**
     * @brief 计算概率密度
     * @param x 输入值
     * @return 混合概率密度
     */
    double probabilityDensity(double x) const;

    /**
     * @brief 获取样本的各分量后验概率
     * @param x 输入值
     * @return 各分量概率向量
     */
    QVector<double> componentProbs(double x) const;

    /**
     * @brief 生成随机样本(从拟合模型采样)
     * @param count 样本数
     * @return 生成的样本
     */
    QVector<double> sample(int count) const;

    /** @brief 获取当前拟合结果 */
    FitResult currentResult() const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param k 分量数 @param ll 对数似然 */
    void fitCompleted(int k, double ll);

    /** @brief 迭代进度 @param iteration 当前迭代 @param ll 当前对数似然 */
    void iterationProgress(int iteration, double ll);

private:
    /** @brief E步: 计算后验概率 */
    void expectationStep(const QVector<double>& data);

    /** @brief M步: 更新参数 */
    void maximizationStep(const QVector<double>& data);

    /** @brief 计算对数似然 */
    double computeLogLikelihood(const QVector<double>& data) const;

    /** @brief 计算单分量高斯概率 */
    double gaussianPdf(double x, double mean, double variance) const;

    /** @brief 初始化分量参数(K-means++) */
    void initializeComponents(const QVector<double>& data);

    /** @brief 计算BIC */
    double computeBIC(double ll, int n, int k) const;

    /** @brief 计算AIC */
    double computeAIC(double ll, int k) const;

    int m_k = 3;                         ///< 分量数
    int m_maxIterations = 100;           ///< 最大迭代次数
    double m_tolerance = 1e-6;           ///< 收敛阈值
    FitResult m_currentResult;           ///< 当前拟合结果
    bool m_fitted = false;               ///< 是否已拟合

    mutable Stats m_stats;         ///< 操作统计
    mutable double m_timeSum = 0.0;///< 累计耗时
};

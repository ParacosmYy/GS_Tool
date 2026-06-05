/**
 * @file GaussianMixture4.h
 * @brief 高斯混合模型(GMM) — EM算法/Dirichlet先验/模型选择
 *
 * 功能: 支持带Dirichlet先验的EM聚类，BIC/AIC模型选择，
 *       以及Collapsed Gibbs Sampling推断，适用于串口数据的
 *       多模态分布建模与异常检测。
 *
 * 协作: DataClassifier(分类) / AnomalyDetector(异常检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型 — 带正则化的EM聚类与模型选择
 */
class GaussianMixture4 : public QObject {
    Q_OBJECT

public:
    /** @brief 单个高斯分量参数 */
    struct Component {
        double weight = 0.0;           ///< 混合权重 π_k
        double mean = 0.0;             ///< 均值 μ_k
        double variance = 1.0;         ///< 方差 σ²_k
        QVector<double> responsibilities; ///< 后验责任 γ(z_nk)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFits = 0;             ///< 累计拟合次数
        quint64 totalSamplesProcessed = 0; ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        double  bestBic = 0.0;             ///< 最优BIC值
        int     bestK = 0;                 ///< 最优分量数
    };

    explicit GaussianMixture4(QObject* parent = nullptr);

    /** @brief 设置分量数 @param k 分量数(≥1) */
    void setK(int k);

    /** @brief 设置Dirichlet先验浓度参数 @param alpha 浓度参数(>0) */
    void setDirichletAlpha(double alpha);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛阈值 @param tol 对数似然变化阈值 */
    void setTolerance(double tol);

    /** @brief EM拟合 @param data 样本数据 @return 最终对数似然 */
    double fit(const QVector<double>& data);

    /** @brief 预测样本属于哪个分量 @param x 样本值 @return 分量索引 */
    int predict(double x) const;

    /** @brief 预测每个分量的后验概率 @param x 样本值 @return 责任向量 */
    QVector<double> predictProba(double x) const;

    /** @brief BIC模型选择 @param data 样本 @param kMin 最小k @param kMax 最大k @return 最优k */
    int selectModelBIC(const QVector<double>& data, int kMin = 1, int kMax = 6);

    /** @brief AIC模型选择 @param data 样本 @param kMin 最小k @param kMax 最大k @return 最优k */
    int selectModelAIC(const QVector<double>& data, int kMin = 1, int kMax = 6);

    /** @brief Collapsed Gibbs采样 @param data 样本 @param burnIn 燃烧期 @param samples 采样数 */
    void gibbsSample(const QVector<double>& data, int burnIn = 100, int samples = 500);

    /** @brief 获取分量参数 @return 分量列表 */
    const QVector<Component>& components() const { return m_components; }

    /** @brief 计算概率密度 @param x 样本值 @return 混合密度p(x) */
    double density(double x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param logLikelihood 最终对数似然 @param iterations 迭代次数 */
    void fitCompleted(double logLikelihood, int iterations);

    /** @brief 模型选择完成 @param bestK 最优分量数 @param score 最优得分 */
    void modelSelected(int bestK, double score);

private:
    double gaussianPdf(double x, double mean, double variance) const;
    double computeLogLikelihood(const QVector<double>& data) const;
    double computeBIC(const QVector<double>& data, int k, double ll) const;
    double computeAIC(const QVector<double>& data, int k, double ll) const;
    void initializeComponents(const QVector<double>& data);
    double eStep(const QVector<double>& data);
    void mStep(const QVector<double>& data);

    int m_k;                       ///< 分量数
    double m_alpha;                ///< Dirichlet浓度参数
    int m_maxIter;                 ///< 最大迭代次数
    double m_tol;                  ///< 收敛阈值
    QVector<Component> m_components; ///< 高斯分量列表

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};

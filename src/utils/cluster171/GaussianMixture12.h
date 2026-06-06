/**
 * @file GaussianMixture12.h
 * @brief 高斯混合模型(对角/满协方差切换+AIC/BIC模型选择) — GMM with Diagonal/Full Covariance Switching and AIC/BIC Model Selection
 *
 * 功能: 实现高斯混合模型EM聚类，支持对角/满协方差自动切换、
 *       AIC/BIC准则模型选择、对数似然监控。
 *
 * 协作: SubspaceCluster4(子空间聚类) / KMeans15(K-Means) / DBSCAN9(DBSCAN)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型聚类器
 */
class GaussianMixture12 : public QObject {
    Q_OBJECT

public:
    /** @brief 协方差类型 */
    enum CovarianceType { Diagonal = 0, Full = 1 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;            ///< 累计运行次数
        int lastComponents = 0;           ///< 最近使用的分量数
        double lastLogLikelihood = 0.0;   ///< 最近对数似然
        double lastAic = 0.0;             ///< 最近AIC
        double lastBic = 0.0;             ///< 最近BIC
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit GaussianMixture12(QObject *parent = nullptr);
    ~GaussianMixture12() override;

    void setMaxComponents(int k);
    void setCovarianceType(CovarianceType type);
    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /**
     * @brief 执行GMM聚类
     * @param data 数据集(每行一个样本)
     * @return 每个样本的簇标签
     */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 自动模型选择(AIC/BIC) */
    QVector<int> fitAuto(const QVector<QVector<double>>& data,
                         int maxK = 10);

    /** @brief 获取分量权重 */
    QVector<double> weights() const;

    /** @brief 获取分量均值 */
    QVector<QVector<double>> means() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, double logLikelihood);
    void modelSelected(int bestK, double aic, double bic);

private:
    /** @brief 单个高斯分量 */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        QVector<double> diagVar;      ///< 对角方差
        QVector<QVector<double>> cov; ///< 满协方差
        QVector<QVector<double>> covInv; ///< 协方差逆(满)
        double logDet = 0.0;
    };

    /** @brief 初始化分量(K-Means++) */
    void initComponents(const QVector<QVector<double>>& data, int k);

    /** @brief E步: 计算后验概率 */
    void eStep(const QVector<QVector<double>>& data,
               QVector<QVector<double>>& resp);

    /** @brief M步: 更新参数 */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief 计算对数似然 */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief 计算单点对数概率密度 */
    double logPdf(int comp, const QVector<double>& x) const;

    /** @brief 计算AIC */
    double computeAIC(double ll, int k, int d) const;

    /** @brief 计算BIC */
    double computeBIC(double ll, int k, int d, int n) const;

    /** @brief 更新满协方差逆 */
    void updateInverse(int comp, int d);

    int m_maxK = 5;
    CovarianceType m_covType = Diagonal;
    int m_maxIter = 200;
    double m_tol = 1e-6;

    QVector<Component> m_components;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file GaussianMixture.h
 * @brief 高斯混合模型 — EM算法拟合多高斯分布
 *
 * 功能: 使用期望最大化(EM)算法拟合高斯混合模型，
 *       支持多分量概率预测和责任度计算。
 *
 * 协作: KernelPca(降维预处理) / CrossValidator(模型验证)
 */
#ifndef GAUSSIANMIXTURE_H
#define GAUSSIANMIXTURE_H

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型 — EM算法
 */
class GaussianMixture : public QObject {
    Q_OBJECT

public:
    /** @brief 单个高斯分量参数 */
    struct Component {
        double mean = 0.0;          ///< 均值
        double variance = 1.0;      ///< 方差
        double weight = 0.0;        ///< 混合权重
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalFits = 0;              ///< 累计拟合次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit GaussianMixture(QObject* parent = nullptr);

    /**
     * @brief 拟合高斯混合模型
     * @param data 输入数据
     * @param k 分量数
     * @param maxIter 最大EM迭代次数
     * @return 最终对数似然
     */
    double fit(const QVector<double>& data, int k, int maxIter = 100);

    /**
     * @brief 预测样本属于哪个分量
     * @param sample 输入样本
     * @return 分量索引
     */
    int predict(double sample) const;

    /**
     * @brief 计算样本对各分量的责任度(后验概率)
     * @param sample 输入样本
     * @return 各分量责任度
     */
    QVector<double> responsibilities(double sample) const;

    /** @brief 获取拟合的分量参数 */
    const QVector<Component>& components() const { return m_components; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param k 分量数 @param logLikelihood 最终对数似然 */
    void fitCompleted(int k, double logLikelihood);

private:
    /** @brief 计算单个高斯PDF @param x 样本 @param comp 分量参数 */
    double gaussianPdf(double x, const Component& comp) const;

    /** @brief E步 — 计算责任度 */
    void eStep(const QVector<double>& data,
               QVector<QVector<double>>& resp);

    /** @brief M步 — 更新参数 @param data 数据 @param resp 责任度 */
    void mStep(const QVector<double>& data,
               const QVector<QVector<double>>& resp);

    /** @brief 计算对数似然 @param data 数据 */
    double logLikelihood(const QVector<double>& data) const;

    QVector<Component> m_components;    ///< 高斯分量列表
    mutable Stats m_stats;              ///< 统计信息
    mutable double m_timeSum = 0.0;      ///< 累计耗时
};

#endif // GAUSSIANMIXTURE_H

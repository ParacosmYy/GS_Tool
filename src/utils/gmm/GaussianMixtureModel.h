/**
 * @file GaussianMixtureModel.h
 * @brief 高斯混合模型 — EM聚类/密度估计
 *
 * 功能: 实现GMM的EM训练/概率密度估计/分量分配，
 *       统计训练/推断次数/迭代数/耗时。
 */
#ifndef GAUSSIANMIXTUREMODEL_H
#define GAUSSIANMIXTUREMODEL_H

#include <QObject>
#include <QVector>

class GaussianMixtureModel : public QObject {
    Q_OBJECT
public:
    /** 单个高斯分量 */
    struct Component {
        double mean = 0.0;      ///< 均值
        double variance = 1.0;  ///< 方差
        double weight = 0.0;    ///< 混合权重
    };

    /** 模型统计 */
    struct Stats {
        quint64 totalTrainings = 0;
        quint64 totalPredictions = 0;
        quint64 totalIterationsUsed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit GaussianMixtureModel(QObject* parent = nullptr);

    /** @brief 设置分量数 @param k 分量数 */
    void setComponents(int k);

    /** @brief 训练 @param data 1D数据 @param maxIterations 最大EM迭代 @param tolerance 收敛阈值 @return 最终log似然 */
    double train(const QVector<double>& data,
                 int maxIterations = 100, double tolerance = 1e-6);

    /** @brief 概率密度 @param x 输入值 @return p(x) */
    double probabilityDensity(double x) const;

    /** @brief 分量归属 @param x 输入值 @return 最可能分量索引 */
    int assignComponent(double x) const;

    /** @brief 获取分量 @return 分量列表 */
    const QVector<Component>& components() const { return m_components; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void trainingCompleted(int iterations, double logLikelihood);

private:
    /** 高斯PDF */
    static double gaussianPdf(double x, double mean, double variance);
    /** 初始化分量(K-means++) */
    void initialize(const QVector<double>& data);

    int m_k;
    QVector<Component> m_components;
    Stats m_stats;
    double m_timeSum;
};

#endif // GAUSSIANMIXTUREMODEL_H

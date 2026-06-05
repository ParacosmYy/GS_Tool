#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GaussianMixture13 - 高斯混合模型聚类
 *
 * 使用EM算法拟合高斯混合模型，支持全/对角/球面
 * 协方差类型，提供软聚类和模型选择(BIC/AIC)。
 */
class GaussianMixture13 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFits = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture13(QObject* parent = nullptr);

    /** @brief 指定分量数和协方差类型拟合模型 */
    bool fit(const QVector<QVector<double>>& data, int components, const QString& covType = "full");

    /** @brief 预测样本属于各分量的后验概率 */
    QVector<double> predictProba(const QVector<double>& sample) const;

    /** @brief 预测样本的最可能分量 */
    int predict(const QVector<double>& sample) const;

    /** @brief 计算数据的BIC分数用于模型选择 */
    double bicScore() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int iterations, double logLikelihood);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_components = 0;
    double m_bic = 0.0;
};

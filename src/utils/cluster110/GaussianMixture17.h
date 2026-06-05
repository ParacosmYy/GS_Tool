#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型(GMM)聚类实现
 *
 * 基于期望最大化(EM)算法拟合多分量高斯分布，支持对角/全协方差矩阵，
 * 提供软聚类概率输出，适用于模式分类、异常检测和语音识别特征建模。
 */
class GaussianMixture17 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalFitted = 0; double avgProcessingTimeMs = 0.0; };

    explicit GaussianMixture17(QObject* parent = nullptr);

    /** @brief 设置高斯分量数量K */
    void setComponentCount(int k);

    /** @brief 设置协方差类型："diag"对角或"full"满矩阵 */
    void setCovarianceType(const QString& type);

    /** @brief 设置EM算法最大迭代次数和收敛阈值 */
    void setEMParams(int maxIterations, double tolerance);

    /** @brief 对输入数据拟合GMM，返回每个样本的簇概率分布 */
    QVector<QVector<double>> fitPredict(const QVector<QVector<double>>& data);

    /** @brief 计算新样本在各分量下的对数似然值 */
    QVector<double> scoreSamples(const QVector<QVector<double>>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 拟合完成信号，返回分量数和对数似然 */
    void fittingCompleted(int components, double logLikelihood);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_components = 3;
    QString m_covType = QStringLiteral("full");
    int m_maxIter = 100;
    double m_tolerance = 1e-6;
    QVector<QVector<double>> m_means;
    QVector<QVector<double>> m_variances;
    QVector<double> m_weights;
};

#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(GMM)聚类器
 *
 * 基于期望最大化(EM)算法拟合多元高斯分量混合分布,
 * 适用于软聚类、概率密度估计与异常检测。
 */
class GaussianMixture15 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit GaussianMixture15(QObject* parent = nullptr);

    /** @brief 设置高斯分量数 */
    void setComponentCount(int count);

    /** @brief 设置EM最大迭代次数 */
    void setMaxIter(int maxIter);

    /** @brief 对输入数据拟合高斯混合模型 */
    void fit(const QVector<QVector<double>>& data);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 聚类完成信号,返回分量数 */
    void clusteringCompleted(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_componentCount = 3;
    int m_maxIter = 100;
};

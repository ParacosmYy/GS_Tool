#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型(GMM)聚类实现
 *
 * 基于EM(期望最大化)算法拟合多高斯分量混合分布，
 * 可进行软聚类和概率密度估计。
 */
class GaussianMixture16 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalClustered = 0; double avgProcessingTimeMs = 0.0; };

    explicit GaussianMixture16(QObject* parent = nullptr);

    /** @brief 设置高斯分量数量 */
    void setComponentCount(int count);

    /** @brief 设置EM最大迭代次数 */
    void setMaxIter(int maxIter);

    /** @brief 对输入数据拟合高斯混合模型 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 聚类完成信号 */
    void clusteringCompleted(int componentCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_componentCount = 3;
    int m_maxIter = 100;
};

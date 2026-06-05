#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AffinityProp4 - 近邻传播聚类算法
 *
 * 基于消息传递的聚类方法，自动确定簇数量，
 * 通过responsibility和availability迭代收敛。
 */
class AffinityProp4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalIterations = 0;
        int totalClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AffinityProp4(QObject* parent = nullptr);

    /** @brief 设置偏好值(影响簇数量) */
    void setPreference(double preference);

    /** @brief 设置阻尼系数(0.5~1.0，防止震荡) */
    void setDamping(double damping);

    /** @brief 从相似度矩阵执行聚类 */
    QVector<int> fit(const QVector<QVector<double>>& similarity, int maxIterations = 200);

    /** @brief 获取簇中心(代表点)索引 */
    QVector<int> exemplars() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_preference = 0.0;
    double m_damping = 0.5;
    QVector<int> m_exemplars;
};

/**
 * @file AffinityCluster.h
 * @brief 亲和力传播聚类 — 消息传递算法
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 亲和力传播聚类引擎
 * 通过responsibility/availability消息传递自动确定簇数
 */
class AffinityCluster : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;               ///< 累计拟合次数
        int totalIterations = 0;         ///< 累计迭代次数
        int totalExemplars = 0;          ///< 累计exemplar数
        double avgProcessingTimeMs = 0.0;
    };

    explicit AffinityCluster(QObject* parent = nullptr);

    /** @brief 拟合数据 @param similarity N×N相似度矩阵 @param preference 对角线偏好值 @param maxIter 最大迭代 @param damping 阻尼系数 */
    void fit(const QVector<QVector<double>>& similarity,
             double preference = 0.0, int maxIter = 200, double damping = 0.5);

    /** @brief 获取聚类标签 @return 每个点的exemplar索引 */
    QVector<int> labels() const { return m_labels; }

    /** @brief 获取exemplar索引 */
    QVector<int> exemplars() const { return m_exemplars; }

    /** @brief 获取簇数 */
    int clusterCount() const { return m_exemplars.size(); }

    /** @brief 获取残差(收敛度量) */
    double residual() const { return m_residual; }

    /** @brief 获取迭代次数 */
    int iterations() const { return m_iterations; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 迭代完成 @param iter 当前迭代 @param residual 残差 */
    void iterationCompleted(int iter, double residual);
    /** @brief 拟合完成 @param clusters 簇数 */
    void fitCompleted(int clusters);

private:
    int m_n = 0;                          ///< 数据点数
    int m_iterations = 0;                 ///< 实际迭代次数
    double m_residual = 0.0;             ///< 最终残差
    QVector<int> m_labels;                ///< 聚类标签
    QVector<int> m_exemplars;             ///< exemplar索引

    Stats m_stats;
    double m_timeSum = 0.0;
};

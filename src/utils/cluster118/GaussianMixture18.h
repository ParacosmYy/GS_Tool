#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GaussianMixture18 - 高斯混合模型第18代实现
 *
 * 提供GMM的参数估计与概率密度计算，支持EM算法迭代、
 * BIC/AIC模型选择、变分推断及在线参数更新。
 */
class GaussianMixture18 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFitOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaussianMixture18(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 使用EM算法拟合GMM参数
     * @param dataPoints 输入数据点集合
     * @param numComponents 高斯分量数
     * @param maxIterations 最大迭代次数
     * @return 各数据点的软分配概率
     */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& dataPoints,
                                 int numComponents, int maxIterations = 100);

    /**
     * @brief 计算给定数据点的概率密度
     * @param point 数据点
     * @return 概率密度值
     */
    double probabilityDensity(const QVector<double>& point);

    /**
     * @brief 预测数据点最可能属于的分量
     * @param dataPoints 数据点集合
     * @return 各点的分量标签
     */
    QVector<int> predict(const QVector<QVector<double>>& dataPoints);

    /**
     * @brief 使用BIC准则选择最优分量数
     * @param dataPoints 数据点集合
     * @param maxComponents 最大候选分量数
     * @return 最优分量数
     */
    int selectComponentsBIC(const QVector<QVector<double>>& dataPoints,
                            int maxComponents = 10);

    /**
     * @brief 在线更新GMM参数（单点增量学习）
     * @param newPoint 新数据点
     * @param learningRate 学习率
     */
    void onlineUpdate(const QVector<double>& newPoint, double learningRate = 0.01);

signals:
    void fittingCompleted(int iterationCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

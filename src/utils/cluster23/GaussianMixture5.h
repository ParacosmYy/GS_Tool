/**
 * @file GaussianMixture5.h
 * @brief 变分贝叶斯高斯混合 — 自动确定分量数
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QVector>
#include <QPair>

/**
 * @brief 变分贝叶斯高斯混合模型
 * 通过变分推断自动修剪多余分量
 */
class GaussianMixture5 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 拟合结果 */
    struct FitResult {
        QVector<double> weights;           ///< 分量权重
        QVector<QVector<double>> means;    ///< 分量均值
        QVector<QVector<QVector<double>>> covariances; ///< 协方差矩阵
        QVector<double> lowerBounds;       ///< 变分下界历史
        int activeComponents = 0;          ///< 活跃分量数
        int iterations = 0;               ///< 迭代次数
    };

    explicit GaussianMixture5(int maxComponents = 10, QObject* parent = nullptr);

    /** @brief 拟合数据 @param data N×D矩阵 @param dims 特征维度 @param maxIter 最大迭代 */
    FitResult fit(const QVector<double>& data, int dims, int maxIter = 200);

    /** @brief 预测每个样本的后验分量概率 @return N×K矩阵 */
    QVector<QVector<double>> predict(const QVector<double>& data, int dims) const;

    /** @brief 获取模型参数 */
    const FitResult& model() const { return m_result; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitProgress(int iter, double lowerBound, int active);
    void fitCompleted(int activeComponents);

private:
    int m_maxComponents;
    int m_dims = 0;
    FitResult m_result;
    Stats m_stats;
    double m_timeSum = 0.0;
};

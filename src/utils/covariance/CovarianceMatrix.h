/**
 * @file CovarianceMatrix.h
 * @brief 协方差矩阵计算器 — 多变量统计
 *
 * 功能: 计算协方差矩阵/相关矩阵/马氏距离，
 *       统计计算次数/维度/耗时。
 */
#ifndef COVARIANCEMATRIX_H
#define COVARIANCEMATRIX_H

#include <QObject>
#include <QVector>

class CovarianceMatrix : public QObject {
    Q_OBJECT
public:
    /** 计算统计 */
    struct Stats {
        quint64 totalComputations = 0;
        quint64 totalDimensionsProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit CovarianceMatrix(QObject* parent = nullptr);

    /** @brief 计算协方差矩阵 @param data 每行一个样本,每列一个变量 @param sample 样本还是总体 @return 协方差矩阵 */
    QVector<QVector<double>> compute(const QVector<QVector<double>>& data,
                                     bool sample = true) const;

    /** @brief 计算相关矩阵 @param covMatrix 协方差矩阵 @return 相关系数矩阵 */
    QVector<QVector<double>> correlationMatrix(
        const QVector<QVector<double>>& covMatrix) const;

    /** @brief 马氏距离 @param x 观测向量 @param mean 均值向量 @param covInv 协方差逆 @return 距离 */
    double mahalanobisDistance(const QVector<double>& x,
                               const QVector<double>& mean,
                               const QVector<QVector<double>>& covInv) const;

    /** @brief 计算均值向量 @param data 数据矩阵 @return 均值 */
    QVector<double> computeMean(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(int dimensions);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // COVARIANCEMATRIX_H

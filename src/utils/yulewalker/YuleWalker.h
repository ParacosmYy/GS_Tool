/**
 * @file YuleWalker.h
 * @brief Yule-Walker方程求解 — AR模型参数估计
 *
 * 功能: Yule-Walker方程求解用于AR模型参数估计，
 *       Levinson-Durbin递归实现，支持模型阶数选择(AIC/BIC)，
 *       统计估计次数/耗时。
 */
#ifndef YULEWALKER_H
#define YULEWALKER_H

#include <QObject>
#include <QVector>

class YuleWalker : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalEstimates = 0;
        quint64 totalPredictions = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit YuleWalker(QObject* parent = nullptr);

    /** @brief 估计AR模型参数 @param signal 输入信号 @param order AR阶数 @return AR系数(不含a[0]=1) */
    QVector<double> estimate(const QVector<double>& signal, int order);

    /** @brief 自动选择最佳阶数(AIC) @param signal 输入信号 @param maxOrder 最大阶数 @return {最佳阶数, AR系数} */
    QPair<int, QVector<double>> autoFit(const QVector<double>& signal,
                                         int maxOrder = 50);

    /** @brief 使用AR模型预测 @param signal 已知信号 @param coeffs AR系数 @param steps 预测步数 @return 预测值 */
    QVector<double> predict(const QVector<double>& signal,
                            const QVector<double>& coeffs, int steps);

    /** @brief 计算AIC @param n 样本数 @param order 阶数 @param sigma2 残差方差 @return AIC值 */
    double aic(int n, int order, double sigma2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(int order, double residualVariance);
    void predictionCompleted(int steps);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // YULEWALKER_H

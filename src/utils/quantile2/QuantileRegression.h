/**
 * @file QuantileRegression.h
 * @brief 分位数回归(Quantile Regression)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class QuantileRegression
 * @brief 分位数回归 — 条件分位数估计
 *
 * 支持线性分位数回归、多分位数同时拟合、
 * 置信区间估计。
 * 适用于异方差数据分析、风险估计、尾部预测等场景。
 */
class QuantileRegression : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFitted = 0;      /**< 总拟合次数 */
        int totalPredictions = 0; /**< 总预测次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit QuantileRegression(QObject* parent = nullptr);

    /**
     * @brief 拟合线性分位数回归
     * @param x 自变量(每行一个样本,含截距列)
     * @param y 因变量
     * @param tau 分位数(0~1)
     * @return 回归系数
     */
    QVector<double> fit(const QVector<QVector<double>>& x,
                         const QVector<double>& y, double tau = 0.5);

    /**
     * @brief 多分位数拟合
     * @param x 自变量
     * @param y 因变量
     * @param taus 分位数列表
     * @return 每个分位数的系数
     */
    QVector<QVector<double>> fitMulti(const QVector<QVector<double>>& x,
                                        const QVector<double>& y,
                                        const QVector<double>& taus);

    /**
     * @brief 预测
     * @param coefficients 回归系数
     * @param x 新自变量
     * @return 预测值
     */
    static double predict(const QVector<double>& coefficients,
                           const QVector<double>& x);

    /**
     * @brief 计算检查函数(Check Function)损失
     * @param residuals 残差
     * @param tau 分位数
     * @return 损失值
     */
    static double checkLoss(const QVector<double>& residuals, double tau);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitted(double tau, int coefficients);

private:
    QVector<double> solveLinearQuantile(const QVector<QVector<double>>& x,
                                          const QVector<double>& y,
                                          double tau);

    Stats m_stats;
    double m_timeSum;
};

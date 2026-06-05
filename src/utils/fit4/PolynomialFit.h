/**
 * @file PolynomialFit.h
 * @brief 多项式拟合(最小二乘)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class PolynomialFit
 * @brief 多项式最小二乘拟合
 *
 * 通过法方程(Gauss-Jordan)求解多项式系数。
 * 支持拟合优度评估(R²)和插值预测。
 */
class PolynomialFit : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;          /**< 总拟合次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit PolynomialFit(QObject* parent = nullptr);

    /**
     * @brief 多项式拟合
     * @param x X坐标
     * @param y Y坐标
     * @param degree 多项式阶数
     * @return 多项式系数[a0, a1, a2, ...] (a0 + a1*x + a2*x² + ...)
     */
    QVector<double> fit(const QVector<double>& x, const QVector<double>& y,
                         int degree) const;

    /**
     * @brief 使用拟合结果预测
     * @param coefficients 多项式系数
     * @param x 预测点X
     * @return 预测值Y
     */
    double evaluate(const QVector<double>& coefficients, double x) const;

    /**
     * @brief 批量预测
     * @param coefficients 多项式系数
     * @param xs 预测点列表
     * @return 预测值列表
     */
    QVector<double> evaluateBatch(const QVector<double>& coefficients,
                                   const QVector<double>& xs) const;

    /**
     * @brief 计算R²(决定系数)
     * @param x X坐标
     * @param y Y坐标
     * @param coefficients 拟合系数
     * @return R²值(0~1，越接近1拟合越好)
     */
    double rSquared(const QVector<double>& x, const QVector<double>& y,
                     const QVector<double>& coefficients) const;

    /**
     * @brief 自动选择最佳阶数(AIC准则)
     * @param x X坐标
     * @param y Y坐标
     * @param maxDegree 最大尝试阶数
     * @return 最佳系数
     */
    QVector<double> autoFit(const QVector<double>& x, const QVector<double>& y,
                             int maxDegree = 10) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitCompleted(int degree, double rSquared);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

/**
 * @file PolynomialRegression.h
 * @brief 多项式回归 — 最小二乘法任意阶拟合
 *
 * 功能: 基于正规方程的最小二乘多项式拟合，
 *       支持任意阶数、预测、R²评估，统计拟合次数/阶数/耗时。
 */
#ifndef POLYNOMIALREGRESSION_H
#define POLYNOMIALREGRESSION_H

#include <QObject>
#include <QVector>

class PolynomialRegression : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalFits = 0;
        quint64 totalPredictions = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    /** 拟合结果 */
    struct FitResult {
        QVector<double> coefficients; ///< 从常数项到最高次项
        double rSquared = 0.0;        ///< 决定系数
        double rmse = 0.0;            ///< 均方根误差
        int degree = 0;               ///< 多项式阶数
    };

    explicit PolynomialRegression(QObject* parent = nullptr);

    /** @brief 拟合多项式 @param x X数据 @param y Y数据 @param degree 阶数 @return 拟合结果 */
    FitResult fit(const QVector<double>& x, const QVector<double>& y,
                  int degree);

    /** @brief 使用已有系数预测 @param coefficients 系数 @param xValue X值 @return Y预测 */
    double predict(const QVector<double>& coefficients, double xValue) const;

    /** @brief 批量预测 @param coefficients 系数 @param xValues X值序列 @return Y预测序列 */
    QVector<double> predictBatch(const QVector<double>& coefficients,
                                  const QVector<double>& xValues) const;

    /** @brief 自动选择最佳阶数(AIC) @param x X数据 @param y Y数据 @param maxDegree 最大阶数 @return 最佳拟合结果 */
    FitResult autoFit(const QVector<double>& x, const QVector<double>& y,
                      int maxDegree = 10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int degree, double rSquared);

private:
    /** 求解线性方程组(高斯消元) */
    QVector<double> solveSystem(QVector<QVector<double>>& A,
                                QVector<double>& b) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // POLYNOMIALREGRESSION_H

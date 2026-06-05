/**
 * @file RegressionEngine.h
 * @brief 回归引擎 — 多种回归分析方法
 *
 * 功能: 支持线性/多项式/对数/指数/幂回归，自动选择最优模型，
 *       统计拟合次数/平均R²/平均RMSE。
 */
#ifndef REGRESSIONENGINE_H
#define REGRESSIONENGINE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @class RegressionEngine
 * @brief 通用回归分析引擎
 */
class RegressionEngine : public QObject {
    Q_OBJECT
public:
    /** 回归类型 */
    enum class RegressionType {
        Linear,         ///< y = ax + b
        Polynomial,     ///< y = aₙxⁿ + ... + a₁x + a₀
        Logarithmic,    ///< y = a·ln(x) + b
        Exponential,    ///< y = a·e^(bx)
        Power           ///< y = a·x^b
    };

    /** 回归结果 */
    struct RegressionResult {
        RegressionType type;
        QVector<double> coefficients;   ///< 系数向量
        double rSquared = 0.0;          ///< 决定系数R²
        double adjustedRSquared = 0.0;  ///< 调整后R²
        double rmse = 0.0;              ///< 均方根误差
        double mae = 0.0;               ///< 平均绝对误差
        double slope = 0.0;             ///< 斜率(线性时)
        double intercept = 0.0;         ///< 截距(线性时)
    };

    /** 引擎统计 */
    struct Stats {
        quint64 totalRegressions = 0;
        double  avgRSquared = 0.0;
        double  avgRmse = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit RegressionEngine(QObject* parent = nullptr);

    void setPolynomialDegree(int degree);

    /** 执行回归 */
    RegressionResult fit(const QVector<double>& x, const QVector<double>& y, RegressionType type);

    /** 自动选择最优模型 */
    RegressionResult autoFit(const QVector<double>& x, const QVector<double>& y);

    /** 使用结果进行预测 */
    double predict(const RegressionResult& model, double x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void regressionComplete(const RegressionResult& result);
    void modelSelected(RegressionType bestType, double rSquared);

private:
    RegressionResult linearRegression(const QVector<double>& x, const QVector<double>& y);
    RegressionResult polyRegression(const QVector<double>& x, const QVector<double>& y);
    RegressionResult logRegression(const QVector<double>& x, const QVector<double>& y);
    RegressionResult expRegression(const QVector<double>& x, const QVector<double>& y);
    RegressionResult powerRegression(const QVector<double>& x, const QVector<double>& y);

    int m_polyDegree;
    Stats m_stats;
    double m_timeSum;
};

#endif // REGRESSIONENGINE_H

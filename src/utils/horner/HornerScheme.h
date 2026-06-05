/**
 * @file HornerScheme.h
 * @brief Horner多项式求值引擎 — 高效多项式计算与导数
 *
 * 功能: 使用Horner法则对多项式进行高效求值，同时支持计算多项式值
 *       和一阶导数。Horner法则将O(n²)的朴素多项式计算降为O(n)。
 *
 * 协作: NewtonRaphson(根求解中多项式求值) / WaveformGenerator(波形拟合)
 */
#ifndef HORNERSCHEME_H
#define HORNERSCHEME_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Horner多项式求值引擎
 *
 * 系数约定: coeffs[0]为最高次项，coeffs[n]为常数项。
 * 即 P(x) = coeffs[0]*x^n + coeffs[1]*x^(n-1) + ... + coeffs[n]
 */
class HornerScheme : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEvaluations = 0;       ///< 累计求值次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit HornerScheme(QObject* parent = nullptr);

    /**
     * @brief 使用Horner法则计算多项式值
     * @param coeffs 多项式系数(从高次到低次)
     * @param x 求值点
     * @return 多项式在x处的值
     */
    double evaluate(const QVector<double>& coeffs, double x);

    /**
     * @brief 同时计算多项式值和一阶导数
     * @param coeffs 多项式系数(从高次到低次)
     * @param x 求值点
     * @return QPair(多项式值, 导数值)
     */
    QPair<double, double> evaluateWithDerivative(
        const QVector<double>& coeffs, double x);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求值完成信号 @param result 多项式值 */
    void evaluationCompleted(double result);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // HORNERSCHEME_H

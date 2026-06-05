/**
 * @file HornerScheme.h
 * @brief Horner法则增强版 — 多项式求值/导数/降次
 *
 * 功能: 使用Horner法则进行高效多项式求值，支持计算任意阶导数
 *       和综合除法(降次)。所有方法均O(n)复杂度。
 *
 * 协作: NewtonRaphson(根求解) / PolynomialRoot(根隔离)
 */
#ifndef HORNERSCHEME2_H
#define HORNERSCHEME2_H

#include <QObject>
#include <QVector>

/**
 * @brief Horner法则增强求值器
 *
 * 系数约定: coeffs[0]为最高次项，coeffs[n]为常数项。
 * P(x) = coeffs[0]*x^n + coeffs[1]*x^(n-1) + ... + coeffs[n]
 */
class HornerEval : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEvaluated = 0;        ///< 累计求值次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit HornerEval(QObject* parent = nullptr);

    /**
     * @brief 使用Horner法则计算多项式值
     * @param coeffs 多项式系数(从高次到低次)
     * @param x 求值点
     * @return 多项式在x处的值
     */
    double evaluate(const QVector<double>& coeffs, double x);

    /**
     * @brief 计算一阶导数在x处的值
     * @param coeffs 多项式系数(从高次到低次)
     * @param x 求值点
     * @return 导数值
     */
    double derivative(const QVector<double>& coeffs, double x);

    /**
     * @brief 计算所有阶导数在x处的值
     * @param coeffs 多项式系数(从高次到低次)
     * @param x 求值点
     * @return 各阶导数值 [P(x), P'(x), P''(x), ...]
     */
    QVector<double> allDerivatives(const QVector<double>& coeffs, double x);

    /**
     * @brief 综合除法 — 用已知根降次
     * @param coeffs 原多项式系数(从高次到低次)
     * @param root 已知根
     * @return 降次后的多项式系数(次数减1)
     */
    QVector<double> deflate(const QVector<double>& coeffs, double root);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求值完成信号 @param degree 多项式次数 */
    void evaluated(int degree);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs = 0.0;   ///< 累计耗时(ms)
};

#endif // HORNERSCHEME2_H

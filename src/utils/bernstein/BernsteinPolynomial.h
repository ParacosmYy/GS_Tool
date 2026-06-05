/**
 * @file BernsteinPolynomial.h
 * @brief Bernstein多项式 — Bezier曲线基函数
 *
 * 功能: 计算Bernstein基函数和Bezier近似。
 *       用于曲线设计和函数逼近。
 *
 * 协作: BezierSpline(贝塞尔曲线) / CubicInterpolator(样条)
 */
#ifndef BERNSTEINPOLYNOMIAL_H
#define BERNSTEINPOLYNOMIAL_H

#include <QObject>
#include <QVector>

/**
 * @brief Bernstein多项式
 */
class BernsteinPolynomial : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEvaluations = 0;   ///< 累计求值次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit BernsteinPolynomial(QObject* parent = nullptr);

    /** @brief 计算Bernstein基函数B_{i,n}(t)
     *  @param t 参数[0,1]
     *  @param i 基函数索引
     *  @param n 多项式次数
     *  @return 基函数值 */
    double basis(double t, int i, int n) const;

    /** @brief 在t处求值n次Bernstein多项式
     *  @param t 参数[0,1]
     *  @param degree 多项式次数
     *  @return 所有基函数值 */
    QVector<double> evaluate(double t, int degree) const;

    /** @brief 用Bernstein多项式逼近数据
     *  @param data 输入数据(均匀采样)
     *  @param degree 逼近次数
     *  @return 逼近系数(控制点) */
    QVector<double> approximate(const QVector<double>& data,
                                int degree);

    /** @brief 从控制点重建曲线
     *  @param controlPoints 控制点
     *  @param nSamples 输出采样数
     *  @return 重建曲线 */
    QVector<double> reconstruct(
        const QVector<double>& controlPoints, int nSamples) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求值完成 @param value 逼近值 */
    void evaluationCompleted(double value);

private:
    /** @brief 组合数C(n,k) */
    double binomial(int n, int k) const;

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // BERNSTEINPOLYNOMIAL_H

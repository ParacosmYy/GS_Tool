/**
 * @file TaylorSeries.h
 * @brief Taylor/Maclaurin级数逼近
 *
 * 功能: 数值计算Taylor级数系数和逼近值。
 *       支持任意阶展开和数值微分。
 *
 * 协作: RungeKuttaSolver(ODE) / HermiteInterpolation(插值)
 */
#ifndef TAYLORSERIES_H
#define TAYLORSERIES_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Taylor级数逼近
 */
class TaylorSeries : public QObject {
    Q_OBJECT

public:
    /** @brief 函数类型 */
    using Func = std::function<double(double)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEvaluations = 0;   ///< 累计求值次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit TaylorSeries(QObject* parent = nullptr);

    /** @brief 使用Taylor级数逼近函数值
     *  @param func 原函数
     *  @param center 展开中心
     *  @param order 展开阶数
     *  @param x 查询点
     *  @return 逼近值 */
    double approximate(const Func& func, double center,
                       int order, double x);

    /** @brief 计算Taylor级数系数(数值微分)
     *  @param func 原函数
     *  @param center 展开中心
     *  @param order 展开阶数
     *  @return 系数数组 a_k = f^{(k)}(center)/k! */
    QVector<double> coefficients(const Func& func,
                                 double center, int order);

    /** @brief 从系数求值
     *  @param coeffs Taylor系数
     *  @param center 展开中心
     *  @param x 查询点
     *  @return 逼近值 */
    double evaluate(const QVector<double>& coeffs,
                    double center, double x) const;

    /** @brief 常用函数的精确Taylor系数 */
    static QVector<double> sinCoeffs(int order);
    static QVector<double> expCoeffs(int order);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求值完成 @param order 阶数 @param value 逼近值 */
    void evaluationCompleted(int order, double value);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // TAYLORSERIES_H

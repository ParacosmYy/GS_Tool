/**
 * @file ChebyshevPoly.h
 * @brief Chebyshev多项式逼近与插值 — 函数逼近/数据拟合/数值积分
 *
 * 功能: 使用Chebyshev多项式进行函数逼近和插值，支持节点计算、
 *       系数求解、Clenshaw递推求值、导数计算和积分计算，
 *       适用于传感器数据拟合、信号逼近、数值分析等场景。
 *
 * 协作: DataInterpolator(数据插值) / DataNormalizer(数据归一化)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chebyshev多项式逼近引擎
 *
 * 典型用法:
 * @code
 *   ChebyshevPoly poly;
 *   poly.fitFromPoints(xs, ys, 8);
 *   double y = poly.evaluate(0.5);
 * @endcode
 */
class ChebyshevPoly : public QObject {
    Q_OBJECT

public:
    /** @brief 逼近结果 */
    struct FitResult {
        QVector<double> coefficients;       ///< Chebyshev系数(T0..Tn)
        double maxError = 0.0;              ///< 最大逼近误差
        double rmsError = 0.0;              ///< 均方根误差
        int degree = 0;                     ///< 多项式阶数
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalFits = 0;                      ///< 累计拟合次数
        int totalEvaluations = 0;               ///< 累计求值次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        int totalDerivatives = 0;               ///< 累计导数计算次数
        int totalIntegrations = 0;              ///< 累计积分计算次数
    };

    explicit ChebyshevPoly(QObject* parent = nullptr);

    /**
     * @brief 从数据点拟合Chebyshev系数
     * @param x X坐标(会被映射到[-1,1])
     * @param y Y坐标
     * @param degree 多项式阶数
     * @return 拟合结果
     */
    FitResult fitFromPoints(const QVector<double>& x,
                            const QVector<double>& y, int degree);

    /**
     * @brief 从函数对象拟合Chebyshev系数
     * @param func 被逼近函数 f(x)->y
     * @param a 区间左端
     * @param b 区间右端
     * @param degree 多项式阶数
     * @return 拟合结果
     */
    FitResult fitFromFunction(const std::function<double(double)>& func,
                              double a, double b, int degree);

    /**
     * @brief 使用Clenshaw递推求值
     * @param x 求值点(原始坐标)
     @ return 函数值
     */
    double evaluate(double x) const;

    /**
     * @brief 批量求值
     * @param xPoints 求值点列表
     * @return 函数值列表
     */
    QVector<double> evaluateBatch(const QVector<double>& xPoints) const;

    /**
     * @brief 计算导数(也用Chebyshev表示)
     * @param x 求值点
     * @return 导数值
     */
    double derivative(double x) const;

    /**
     * @brief 计算定积分
     * @param a 积分下限
     * @param b 积分上限
     * @return 积分值
     */
    double integrate(double a, double b) const;

    /**
     * @brief 计算Chebyshev-Gauss节点
     * @param n 节点数
     * @param a 区间左端
     * @param b 区间右端
     * @return 节点坐标
     */
    static QVector<double> chebyshevNodes(int n, double a, double b);

    /**
     * @brief 计算Chebyshev-Lobatto节点(含端点)
     * @param n 节点数
     * @param a 区间左端
     * @param b 区间右端
     * @return 节点坐标
     */
    static QVector<double> lobattoNodes(int n, double a, double b);

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param result 拟合结果 */
    void fitCompleted(const FitResult& result);

private:
    /** @brief 将x映射到[-1,1] */
    double mapToUnit(double x) const;

    /** @brief 将t从[-1,1]映射回原始区间 */
    double mapFromUnit(double t) const;

    /** @brief Clenshaw递推核心 */
    double clenshaw(double t) const;

    /** @brief 计算导数系数 */
    QVector<double> derivativeCoefficients() const;

    QVector<double> m_coeffs;       ///< Chebyshev系数
    double m_xMin = -1.0;          ///< X区间左端
    double m_xMax = 1.0;           ///< X区间右端
    int m_degree = 0;               ///< 当前阶数
    Stats m_stats;                  ///< 统计数据
    double m_timeSum = 0.0;        ///< 时间累加器
};

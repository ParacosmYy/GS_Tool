/**
 * @file ChebyshevApproximation.h
 * @brief Chebyshev 多项式逼近 — 函数最佳一致逼近
 *
 * 功能: 在 [a,b] 上将目标函数展开为 Chebyshev 级数，
 *       提供系数计算与任意点求值。利用 Clenshaw 递推求值。
 *
 * 协作: RationalInterpolation(有理插值) / DividedDifference(多项式)
 */
#ifndef CHEBYSHEVAPPROXIMATION_H
#define CHEBYSHEVAPPROXIMATION_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Chebyshev 多项式逼近器
 */
class ChebyshevApproximation : public QObject {
    Q_OBJECT

public:
    /** @brief 一元函数类型 */
    using Func = std::function<double(double)>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalApproximations = 0;  ///< 累计逼近次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ChebyshevApproximation(QObject* parent = nullptr);

    /**
     * @brief 计算 Chebyshev 展开系数
     * @param f      目标函数
     * @param a      区间左端点
     * @param b      区间右端点
     * @param degree 多项式阶数
     * @return 系数向量 c[0..degree]
     */
    QVector<double> approximate(Func f, double a, double b, int degree);

    /**
     * @brief 利用 Chebyshev 系数在 x 处求值(Clenshaw 递推)
     * @param coeffs 系数向量
     * @param a      区间左端点
     * @param b      区间右端点
     * @param x      查询点
     * @return 逼近值
     */
    double evaluate(QVector<double> coeffs, double a, double b, double x);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 逼近完成 @param degree 多项式阶数 */
    void approximationCompleted(int degree);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // CHEBYSHEVAPPROXIMATION_H

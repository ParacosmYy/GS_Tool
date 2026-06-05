/**
 * @file PadeApproximant.h
 * @brief Pade逼近 — 有理函数逼近
 *
 * 功能: 从Taylor系数计算Pade逼近的分子/分母系数，
 *       通过求解线性方程组获得有理逼近。
 *
 * 协作: TaylorSeries(Taylor展开) / RationalInterpolation(有理插值)
 */
#ifndef PADEAPPROXIMANT_H
#define PADEAPPROXIMANT_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Pade逼近器
 */
class PadeApproximant : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalApproximations = 0;    ///< 累计逼近次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit PadeApproximant(QObject* parent = nullptr);

    /**
     * @brief 计算Pade逼近系数
     * @param taylorCoeffs Taylor系数(从常数项开始)
     * @param m 分子阶数
     * @param n 分母阶数
     * @return (分子系数, 分母系数)
     */
    QPair<QVector<double>, QVector<double>> approximate(
        const QVector<double>& taylorCoeffs, int m, int n);

    /**
     * @brief 求值Pade逼近
     * @param numer 分子系数
     * @param denom 分母系数
     * @param x 求值点
     * @return 逼近值
     */
    double evaluate(const QVector<double>& numer,
                    const QVector<double>& denom, double x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 逼近完成 @param m 分子阶数 @param n 分母阶数 */
    void approximationCompleted(int m, int n);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // PADEAPPROXIMANT_H

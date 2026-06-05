/**
 * @file PercentileCalculator.h
 * @brief 百分位数/分位数计算器 — 多种插值方法
 *
 * 功能: 计算百分位数/四分位数/箱线图统计量，
 *       支持多种插值方法，统计计算次数/耗时。
 */
#ifndef PERCENTILECALCULATOR_H
#define PERCENTILECALCULATOR_H

#include <QObject>
#include <QVector>

class PercentileCalculator : public QObject {
    Q_OBJECT
public:
    /** 插值方法 */
    enum class Interpolation { Linear, Lower, Higher, Midpoint, Nearest };

    /** 箱线图统计量 */
    struct BoxPlotStats {
        double minimum = 0.0;
        double q1 = 0.0;
        double median = 0.0;
        double q3 = 0.0;
        double maximum = 0.0;
        double iqr = 0.0;
        double lowerFence = 0.0;
        double upperFence = 0.0;
        QVector<double> outliers;
    };

    /** 计算统计 */
    struct Stats {
        quint64 totalComputations = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit PercentileCalculator(QObject* parent = nullptr);

    /** @brief 计算百分位数 @param data 数据 @param p 百分位(0~100) @param method 插值方法 @return 百分位值 */
    double compute(const QVector<double>& data, double p,
                   Interpolation method = Interpolation::Linear);

    /** @brief 计算多个百分位数 @param data 数据 @param percentiles 百分位数列表 @return 百分位值列表 */
    QVector<double> computeMany(const QVector<double>& data,
                                const QVector<double>& percentiles) const;

    /** @brief 箱线图统计量 @param data 数据 @param kIqr IQR倍数 @return 箱线图统计 */
    BoxPlotStats boxPlotStats(const QVector<double>& data,
                              double kIqr = 1.5) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(double percentile, double value);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // PERCENTILECALCULATOR_H

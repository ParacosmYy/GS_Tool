/**
 * @file StatDistribution.h
 * @brief 统计分布分析器 — 检验数据是否符合已知分布
 *
 * 功能: 正态/均匀/指数分布检验，计算PDF/CDF/分位数，
 *       Kolmogorov-Smirnov检验。
 *
 * 协作: HistogramBuilder(分箱后分布拟合) / DataQualityScorer(有效性)
 */
#ifndef STATDISTRIBUTION_H
#define STATDISTRIBUTION_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 统计分布分析器 — 分布拟合与检验
 */
class StatDistribution : public QObject {
    Q_OBJECT

public:
    /** @brief 分布类型 */
    enum class DistributionType {
        Normal,     ///< 正态分布
        Uniform,    ///< 均匀分布
        Exponential,///< 指数分布
        LogNormal   ///< 对数正态分布
    };
    Q_ENUM(DistributionType)

    /** @brief 分布参数 */
    struct DistributionParams {
        double param1 = 0.0;    ///< 参数1(均值/最小值/lambda)
        double param2 = 1.0;    ///< 参数2(标准差/最大值)
        double ksStatistic = 0.0;///< KS统计量
        double pValue = 0.0;   ///< p值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalFitsPerformed = 0;     ///< 累计拟合次数
        quint64 totalTestsPerformed = 0;    ///< 累计检验次数
        double  averageKS = 0.0;            ///< 平均KS统计量
        int     bestFitDistribution = 0;    ///< 最佳拟合分布
    };

    explicit StatDistribution(QObject* parent = nullptr);

    /** @brief 拟合分布参数 @param data 数据 @param type 分布类型 @return 参数 */
    DistributionParams fit(const QVector<double>& data, DistributionType type);

    /** @brief KS检验 @param data 数据 @param params 分布参数 @param type 分布类型 @return KS统计量 */
    double ksTest(const QVector<double>& data,
                  const DistributionParams& params,
                  DistributionType type);

    /** @brief 计算CDF @param x 值 @param params 参数 @param type 分布类型 @return CDF值 */
    double cdf(double x, const DistributionParams& params,
               DistributionType type) const;

    /** @brief 计算PDF @param x 值 @param params 参数 @param type 分布类型 @return PDF值 */
    double pdf(double x, const DistributionParams& params,
               DistributionType type) const;

    /** @brief 计算分位数 @param p 概率(0-1) @param params 参数 @param type 分布类型 @return 分位值 */
    double quantile(double p, const DistributionParams& params,
                    DistributionType type) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

private:
    double normalCDF(double x, double mean, double stddev) const;
    double uniformCDF(double x, double a, double b) const;
    double exponentialCDF(double x, double lambda) const;

    Stats m_stats;
    double m_ksSum;
};

#endif // STATDISTRIBUTION_H

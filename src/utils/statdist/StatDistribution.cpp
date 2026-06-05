/**
 * @file StatDistribution.cpp
 * @brief 统计分布分析器实现 — 分布拟合/KS检验/PDF/CDF
 */

#include "utils/statdist/StatDistribution.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
StatDistribution::StatDistribution(QObject* parent)
    : QObject(parent)
    , m_ksSum(0.0)
{
}

/** @brief 拟合分布参数 @param data 数据 @param type 分布类型 @return 参数 */
StatDistribution::DistributionParams StatDistribution::fit(
    const QVector<double>& data, DistributionType type)
{
    DistributionParams params;
    if (data.isEmpty()) return params;

    int n = data.size();
    double sum = 0.0;
    for (double v : data) sum += v;
    double mean = sum / n;

    switch (type) {
    case DistributionType::Normal: {
        params.param1 = mean;
        double sqSum = 0.0;
        for (double v : data) {
            double d = v - mean;
            sqSum += d * d;
        }
        params.param2 = qSqrt(sqSum / n);
        break;
    }
    case DistributionType::Uniform: {
        params.param1 = *std::min_element(data.begin(), data.end());
        params.param2 = *std::max_element(data.begin(), data.end());
        break;
    }
    case DistributionType::Exponential: {
        params.param1 = 1.0 / mean;
        params.param2 = 0.0;
        break;
    }
    case DistributionType::LogNormal: {
        double logSum = 0.0;
        for (double v : data) {
            logSum += (v > 0) ? qLn(v) : 0.0;
        }
        double logMean = logSum / n;
        double logSqSum = 0.0;
        for (double v : data) {
            double l = (v > 0) ? qLn(v) : 0.0;
            logSqSum += (l - logMean) * (l - logMean);
        }
        params.param1 = logMean;
        params.param2 = qSqrt(logSqSum / n);
        break;
    }
    }

    /* KS检验 */
    params.ksStatistic = ksTest(data, params, type);
    ++m_stats.totalFitsPerformed;
    m_ksSum += params.ksStatistic;
    m_stats.averageKS = m_ksSum
        / static_cast<double>(m_stats.totalFitsPerformed);

    return params;
}

/** @brief KS检验 @param data 数据 @param params 参数 @param type 分布类型 @return KS统计量 */
double StatDistribution::ksTest(
    const QVector<double>& data,
    const DistributionParams& params,
    DistributionType type)
{
    if (data.size() < 2) return 1.0;

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double maxD = 0.0;

    ++m_stats.totalTestsPerformed;

    for (int i = 0; i < n; ++i) {
        double empirical = static_cast<double>(i + 1) / n;
        double theoretical = cdf(sorted[i], params, type);
        double d1 = qAbs(empirical - theoretical);
        double d2 = qAbs(static_cast<double>(i) / n - theoretical);
        maxD = qMax(maxD, qMax(d1, d2));
    }

    return maxD;
}

/** @brief CDF @param x 值 @param params 参数 @param type 类型 @return CDF */
double StatDistribution::cdf(double x,
    const DistributionParams& params, DistributionType type) const
{
    switch (type) {
    case DistributionType::Normal:
        return normalCDF(x, params.param1, params.param2);
    case DistributionType::Uniform:
        return uniformCDF(x, params.param1, params.param2);
    case DistributionType::Exponential:
        return exponentialCDF(x, params.param1);
    case DistributionType::LogNormal:
        return normalCDF((x > 0) ? qLn(x) : -100.0, params.param1, params.param2);
    }
    return 0.0;
}

/** @brief PDF @param x 值 @param params 参数 @param type 类型 @return PDF */
double StatDistribution::pdf(double x,
    const DistributionParams& params, DistributionType type) const
{
    switch (type) {
    case DistributionType::Normal: {
        double d = (x - params.param1) / params.param2;
        return qExp(-0.5 * d * d) / (params.param2 * qSqrt(2.0 * M_PI));
    }
    case DistributionType::Uniform: {
        double range = params.param2 - params.param1;
        return (x >= params.param1 && x <= params.param2)
            ? 1.0 / range : 0.0;
    }
    case DistributionType::Exponential: {
        return (x >= 0) ? params.param1 * qExp(-params.param1 * x) : 0.0;
    }
    case DistributionType::LogNormal: {
        if (x <= 0) return 0.0;
        double lx = qLn(x);
        double d = (lx - params.param1) / params.param2;
        return qExp(-0.5 * d * d) / (x * params.param2 * qSqrt(2.0 * M_PI));
    }
    }
    return 0.0;
}

/** @brief 分位数 @param p 概率 @param params 参数 @param type 类型 @return 分位值 */
double StatDistribution::quantile(double p,
    const DistributionParams& params, DistributionType type) const
{
    p = qBound(0.001, p, 0.999);

    switch (type) {
    case DistributionType::Uniform: {
        return params.param1 + p * (params.param2 - params.param1);
    }
    case DistributionType::Exponential: {
        return -qLn(1.0 - p) / params.param1;
    }
    default: {
        /* 二分搜索 */
        double lo = params.param1 - 10.0 * params.param2;
        double hi = params.param1 + 10.0 * params.param2;
        for (int i = 0; i < 50; ++i) {
            double mid = (lo + hi) / 2.0;
            if (cdf(mid, params, type) < p) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        return (lo + hi) / 2.0;
    }
    }
}

/** @brief 重置统计 */
void StatDistribution::resetStatistics()
{
    m_stats = Stats{};
    m_ksSum = 0.0;
}

/** @brief 正态CDF(近似) @param x 值 @param mean 均值 @param stddev 标准差 @return CDF */
double StatDistribution::normalCDF(double x, double mean, double stddev) const
{
    double z = (x - mean) / stddev;
    double a = qAbs(z);
    double t = 1.0 / (1.0 + 0.2316419 * a);
    double d = 0.3989423 * qExp(-0.5 * a * a);
    double p = d * t * (0.3193815 + t * (-0.3565638 + t
        * (1.781478 + t * (-1.8212560 + t * 1.3302744))));
    return (z > 0) ? 1.0 - p : p;
}

/** @brief 均匀CDF @param x 值 @param a 最小值 @param b 最大值 @return CDF */
double StatDistribution::uniformCDF(double x, double a, double b) const
{
    if (x < a) return 0.0;
    if (x > b) return 1.0;
    return (x - a) / (b - a);
}

/** @brief 指数CDF @param x 值 @param lambda 参数 @return CDF */
double StatDistribution::exponentialCDF(double x, double lambda) const
{
    return (x >= 0) ? 1.0 - qExp(-lambda * x) : 0.0;
}

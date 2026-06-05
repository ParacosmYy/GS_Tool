/**
 * @file DataCorrelator.cpp
 * @brief 数据相关性引擎实现 — Pearson/自相关/互相关/窗口相关
 */

#include "utils/correlator/DataCorrelator.h"

#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DataCorrelator::DataCorrelator(QObject* parent)
    : QObject(parent)
    , m_significanceThreshold(0.05)
    , m_corrSum(0.0)
{
}

/** @brief 设置显著性阈值 @param threshold p-value阈值 */
void DataCorrelator::setSignificanceThreshold(double threshold)
{
    m_significanceThreshold = qBound(0.001, threshold, 0.5);
}

/** @brief Pearson相关系数 @param x 第一组数据 @param y 第二组数据 @return 相关系数 */
double DataCorrelator::pearsonCorrelation(
    const QVector<double>& x, const QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    if (n < 3) {
        return 0.0;
    }

    double sumX = 0.0, sumY = 0.0;
    for (int i = 0; i < n; ++i) {
        sumX += x[i];
        sumY += y[i];
    }
    double meanX = sumX / n;
    double meanY = sumY / n;

    double covXY = 0.0, varX = 0.0, varY = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        covXY += dx * dy;
        varX += dx * dx;
        varY += dy * dy;
    }

    double denom = qSqrt(varX * varY);
    if (qFuzzyIsNull(denom)) {
        return 0.0;
    }

    double r = covXY / denom;
    r = qBound(-1.0, r, 1.0);

    /* 更新统计 */
    ++m_stats.totalCorrelationsComputed;
    m_corrSum += qAbs(r);
    m_stats.averageCorrelation = m_corrSum
        / static_cast<double>(m_stats.totalCorrelationsComputed);
    if (qAbs(r) > m_stats.peakCorrelation) {
        m_stats.peakCorrelation = qAbs(r);
    }

    /* 显著性检验 */
    double pValue = approximatePValue(r, n);
    if (pValue < m_significanceThreshold) {
        ++m_stats.significantPairsFound;
        emit significantCorrelation(r, 0, pValue);
    }

    return r;
}

/** @brief 自相关 @param data 数据 @param lag 滞后阶数 @return 自相关系数 */
double DataCorrelator::autoCorrelation(const QVector<double>& data, int lag)
{
    if (data.size() < lag + 3 || lag < 1) {
        return 0.0;
    }

    int n = data.size();
    double mean = 0.0;
    for (double v : data) {
        mean += v;
    }
    mean /= n;

    double cov = 0.0, var = 0.0;
    for (int i = 0; i < n - lag; ++i) {
        double dx = data[i] - mean;
        cov += dx * (data[i + lag] - mean);
    }
    for (int i = 0; i < n; ++i) {
        double dx = data[i] - mean;
        var += dx * dx;
    }

    if (qFuzzyIsNull(var)) {
        return 0.0;
    }

    double r = cov / var;
    r = qBound(-1.0, r, 1.0);

    ++m_stats.totalCorrelationsComputed;
    m_corrSum += qAbs(r);
    m_stats.averageCorrelation = m_corrSum
        / static_cast<double>(m_stats.totalCorrelationsComputed);

    return r;
}

/** @brief 互相关 @param x 第一组数据 @param y 第二组数据 @param maxLag 最大搜索滞后 @return (最大相关系数, 最优滞后) */
QPair<double, int> DataCorrelator::crossCorrelation(
    const QVector<double>& x, const QVector<double>& y, int maxLag)
{
    int n = qMin(x.size(), y.size());
    if (n < 3) {
        return {0.0, 0};
    }

    double bestR = -2.0;
    int bestLag = 0;

    int searchLag = qMin(maxLag, n - 3);
    for (int lag = -searchLag; lag <= searchLag; ++lag) {
        int start = qMax(0, lag);
        int end = qMin(n, n + lag);
        int count = end - start;

        if (count < 3) continue;

        double sumX = 0.0, sumY = 0.0;
        for (int i = start; i < end; ++i) {
            sumX += x[i];
            sumY += y[i - lag];
        }
        double meanX = sumX / count;
        double meanY = sumY / count;

        double cov = 0.0, varX = 0.0, varY = 0.0;
        for (int i = start; i < end; ++i) {
            double dx = x[i] - meanX;
            double dy = y[i - lag] - meanY;
            cov += dx * dy;
            varX += dx * dx;
            varY += dy * dy;
        }

        double denom = qSqrt(varX * varY);
        if (qFuzzyIsNull(denom)) continue;

        double r = cov / denom;
        r = qBound(-1.0, r, 1.0);

        if (qAbs(r) > qAbs(bestR)) {
            bestR = r;
            bestLag = lag;
        }
    }

    if (bestR < -1.0) {
        bestR = 0.0;
    }

    ++m_stats.totalCorrelationsComputed;
    m_corrSum += qAbs(bestR);
    m_stats.averageCorrelation = m_corrSum
        / static_cast<double>(m_stats.totalCorrelationsComputed);
    if (qAbs(bestR) > m_stats.peakCorrelation) {
        m_stats.peakCorrelation = qAbs(bestR);
    }

    double pValue = approximatePValue(bestR, n);
    if (pValue < m_significanceThreshold) {
        ++m_stats.significantPairsFound;
        emit significantCorrelation(bestR, bestLag, pValue);
    }

    return {bestR, bestLag};
}

/** @brief 滑动窗口相关 @param x 窗口X @param y 窗口Y @return 相关系数 */
double DataCorrelator::windowCorrelation(
    const QVector<double>& x, const QVector<double>& y)
{
    return pearsonCorrelation(x, y);
}

/** @brief 近似p-value @param r 相关系数 @param n 样本数 @return p-value */
double DataCorrelator::approximatePValue(double r, int n)
{
    if (n < 4) return 1.0;

    /* t分布近似 */
    double absR = qAbs(r);
    if (absR >= 1.0) return 0.0;

    double t = absR * qSqrt(static_cast<double>(n - 2) / (1.0 - absR * absR));

    /* 简化的t分布CDF近似(使用正态近似) */
    double df = static_cast<double>(n - 2);
    double z = t / qSqrt(df);
    /* 标准正态CDF近似 */
    double p = 1.0 / (1.0 + 0.2316419 * z);
    p = p * p * p * p * p;
    double cdf = 1.0 - 0.3989423 * qExp(-0.5 * z * z) * p
        * (0.3193815 + p * (-0.3565638 + p * (1.781478
           + p * (-1.8212560 + p * 1.3302744))));

    return qBound(0.0, 2.0 * (1.0 - cdf), 1.0);
}

/** @brief 重置统计 */
void DataCorrelator::resetStatistics()
{
    m_stats = Stats{};
    m_corrSum = 0.0;
}

/**
 * @file CrossCorrelator.cpp
 * @brief 互相关引擎实现 — 标准互相关/归一化互相关/时延检测
 */

#include "utils/crosscorr/CrossCorrelator.h"

#include <QtMath>
#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
CrossCorrelator::CrossCorrelator(QObject* parent)
    : QObject(parent)
    , m_method(Method::Standard)
    , m_maxLag(256)
{
}

/** @brief 设置相关方法 @param method 方法 */
void CrossCorrelator::setMethod(Method method)
{
    m_method = method;
}

/** @brief 设置最大滞后 @param maxLag 最大滞后 */
void CrossCorrelator::setMaxLag(int maxLag)
{
    m_maxLag = qMax(1, maxLag);
}

/** @brief 计算互相关 @param x 第一路 @param y 第二路 @return 结果 */
CrossCorrelator::Result CrossCorrelator::compute(
    const QVector<double>& x, const QVector<double>& y)
{
    if (x.isEmpty() || y.isEmpty()) {
        return Result{};
    }

    QElapsedTimer timer;
    timer.start();

    Result result;
    switch (m_method) {
    case Method::Standard:
        result = computeStandard(x, y);
        break;
    case Method::Normalized:
        result = computeNormalized(x, y);
        break;
    case Method::Phase:
        /* 相位相关回退到归一化方法 */
        result = computeNormalized(x, y);
        break;
    }

    ++m_stats.totalComputations;
    m_stats.totalPointsProcessed += static_cast<quint64>(x.size() + y.size());
    if (qAbs(result.peakValue) > m_stats.peakCorrelation) {
        m_stats.peakCorrelation = qAbs(result.peakValue);
    }
    if (result.peakLag != 0) {
        ++m_stats.totalDelaysDetected;
    }

    emit computationComplete(result.peakLag, result.peakValue);
    return result;
}

/** @brief 检测时延 @param x 第一路 @param y 第二路 @return 时延 */
double CrossCorrelator::detectDelay(const QVector<double>& x,
                                     const QVector<double>& y)
{
    if (x.isEmpty() || y.isEmpty()) return 0.0;

    Result result = compute(x, y);

    /* 抛物线插值细化峰值位置 */
    int k = result.peakLag;
    if (k > 0 && k < result.values.size() - 1) {
        double y0 = result.values[k - 1];
        double y1 = result.values[k];
        double y2 = result.values[k + 1];
        double denom = 2.0 * (2.0 * y1 - y0 - y2);
        if (!qFuzzyIsNull(denom)) {
            result.delay = static_cast<double>(k) + (y0 - y2) / denom;
            return result.delay;
        }
    }
    return static_cast<double>(result.peakLag);
}

/** @brief 计算相似度 @param x 第一路 @param y 第二路 @return 相似度 */
double CrossCorrelator::similarity(const QVector<double>& x,
                                    const QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    if (n == 0) return 0.0;

    double meanX = 0.0, meanY = 0.0;
    for (int i = 0; i < n; ++i) {
        meanX += x[i];
        meanY += y[i];
    }
    meanX /= n;
    meanY /= n;

    double num = 0.0, denX = 0.0, denY = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = x[i] - meanX;
        double dy = y[i] - meanY;
        num += dx * dy;
        denX += dx * dx;
        denY += dy * dy;
    }

    double den = qSqrt(denX * denY);
    if (qFuzzyIsNull(den)) return 0.0;
    return qBound(-1.0, num / den, 1.0);
}

/** @brief 重置统计 */
void CrossCorrelator::resetStatistics()
{
    m_stats = Stats{};
}

/** @brief 标准互相关 @param x 第一路 @param y 第二路 @return 结果 */
CrossCorrelator::Result CrossCorrelator::computeStandard(
    const QVector<double>& x, const QVector<double>& y)
{
    Result result;
    int maxLag = qMin(m_maxLag, qMin(x.size(), y.size()));
    int n = qMin(x.size(), y.size());

    result.lags.resize(2 * maxLag + 1);
    result.values.resize(2 * maxLag + 1);

    for (int lag = -maxLag; lag <= maxLag; ++lag) {
        int idx = lag + maxLag;
        result.lags[idx] = static_cast<double>(lag);

        double sum = 0.0;
        int count = 0;
        for (int i = 0; i < n; ++i) {
            int j = i + lag;
            if (j >= 0 && j < y.size()) {
                sum += x[i] * y[j];
                ++count;
            }
        }
        result.values[idx] = (count > 0) ? sum / count : 0.0;
    }

    /* 找峰值 */
    double peak = -1e300;
    for (int i = 0; i < result.values.size(); ++i) {
        if (result.values[i] > peak) {
            peak = result.values[i];
            result.peakLag = static_cast<int>(result.lags[i]);
        }
    }
    result.peakValue = peak;
    result.delay = static_cast<double>(result.peakLag);

    return result;
}

/** @brief 归一化互相关 @param x 第一路 @param y 第二路 @return 结果 */
CrossCorrelator::Result CrossCorrelator::computeNormalized(
    const QVector<double>& x, const QVector<double>& y)
{
    Result result = computeStandard(x, y);

    /* 归一化到[-1,1] */
    double maxAbs = 0.0;
    for (double v : result.values) {
        if (qAbs(v) > maxAbs) maxAbs = qAbs(v);
    }
    if (maxAbs > 1e-10) {
        for (auto& v : result.values) {
            v /= maxAbs;
        }
        result.peakValue /= maxAbs;
    }

    return result;
}

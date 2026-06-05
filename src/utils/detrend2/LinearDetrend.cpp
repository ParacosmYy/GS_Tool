/**
 * @file LinearDetrend.cpp
 * @brief 线性去趋势实现
 */

#include "utils/detrend2/LinearDetrend.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
LinearDetrend::LinearDetrend(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 对信号进行线性去趋势
 *  @param signal 输入信号
 *  @return 去趋势后的信号 */
QVector<double> LinearDetrend::detrend(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    int n = signal.size();
    if (n < 2) {
        return signal;
    }

    LineFit fit = fitLine(signal);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double trend = fit.slope * static_cast<double>(i) + fit.intercept;
        result[i] = signal[i] - trend;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDetrends;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetrends);

    emit detrendCompleted(n);
    return result;
}

/** @brief 拟合线性趋势线
 *  @param signal 输入信号
 *  @return 拟合结果 */
LinearDetrend::LineFit LinearDetrend::fitLine(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    LineFit result;
    int n = signal.size();

    if (n < 2) {
        result.slope = 0.0;
        result.intercept = (n == 1) ? signal[0] : 0.0;
        result.rSquared = 1.0;

        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalDetrends;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalDetrends);

        return result;
    }

    /* 最小二乘法: y = a*x + b */
    /* x_i = i, 计算sum_x, sum_y, sum_xy, sum_x2 */
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = static_cast<double>(i);
        double y = signal[i];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double nd = static_cast<double>(n);
    double denominator = nd * sumX2 - sumX * sumX;

    if (qAbs(denominator) < 1e-300) {
        result.slope = 0.0;
        result.intercept = sumY / nd;
        result.rSquared = 0.0;
    } else {
        result.slope = (nd * sumXY - sumX * sumY) / denominator;
        result.intercept = (sumY - result.slope * sumX) / nd;

        /* 计算R² */
        double meanY = sumY / nd;
        double ssTot = 0.0, ssRes = 0.0;
        for (int i = 0; i < n; ++i) {
            double x = static_cast<double>(i);
            double yHat = result.slope * x + result.intercept;
            ssRes += (signal[i] - yHat) * (signal[i] - yHat);
            ssTot += (signal[i] - meanY) * (signal[i] - meanY);
        }
        result.rSquared = (ssTot > 1e-300) ? (1.0 - ssRes / ssTot) : 1.0;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDetrends;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetrends);

    return result;
}

/** @brief 重置统计 */
void LinearDetrend::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file HoltWinters.cpp
 * @brief Holt-Winters预测器实现
 */

#include "utils/holtwinters/HoltWinters.h"

#include <QElapsedTimer>
#include <QtMath>

HoltWinters::HoltWinters(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

HoltWinters::FitResult HoltWinters::fit(
    const QVector<double>& data, int seasonLength, ModelType type)
{
    QElapsedTimer timer;
    timer.start();

    FitResult result;
    int n = data.size();
    if (n < 2 * seasonLength) {
        m_stats.totalFits++;
        return result;
    }

    result.level.resize(n);
    result.trend.resize(n);
    result.season.resize(n);

    /* 初始值 */
    result.level[0] = data[0];
    result.trend[0] = (n > seasonLength)
        ? (data[seasonLength] - data[0]) / seasonLength : 0.0;

    /* 初始季节因子 */
    double seasonMean = 0.0;
    for (int i = 0; i < seasonLength && i < n; ++i) seasonMean += data[i];
    seasonMean /= qMin(seasonLength, n);
    for (int i = 0; i < n; ++i) {
        result.season[i] = (seasonMean > 1e-15) ? data[i] / seasonMean : 1.0;
    }

    for (int i = 1; i < n; ++i) {
        int sIdx = qMax(0, i - seasonLength);
        double seasonVal = result.season[sIdx];
        double detrended = (type == ModelType::Multiplicative)
            ? data[i] / qMax(seasonVal, 1e-15)
            : data[i] - seasonVal;

        result.level[i] = result.alpha * detrended +
            (1.0 - result.alpha) * (result.level[i - 1] + result.trend[i - 1]);
        result.trend[i] = result.beta * (result.level[i] - result.level[i - 1]) +
            (1.0 - result.beta) * result.trend[i - 1];

        if (type == ModelType::Multiplicative) {
            result.season[i] = result.gamma * (data[i] / qMax(result.level[i], 1e-15)) +
                (1.0 - result.gamma) * seasonVal;
        } else {
            result.season[i] = result.gamma * (data[i] - result.level[i]) +
                (1.0 - result.gamma) * seasonVal;
        }
    }

    result.mape = computeMape(data, result.level);

    m_stats.totalFits++;
    const auto nFits = m_stats.totalFits;
    m_stats.avgMape = m_stats.avgMape * (nFits - 1) / nFits + result.mape / nFits;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / nFits;

    emit fitCompleted(result.mape);
    return result;
}

QVector<double> HoltWinters::forecast(const FitResult& result, int steps,
                                       int seasonLength) const
{
    QVector<double> predictions;
    predictions.reserve(steps);

    int n = result.level.size();
    if (n == 0) return predictions;

    double lastLevel = result.level.last();
    double lastTrend = result.trend.last();

    for (int h = 1; h <= steps; ++h) {
        int sIdx = qMax(0, n - seasonLength + ((h - 1) % seasonLength));
        double seasonVal = (sIdx < result.season.size())
            ? result.season[sIdx] : 0.0;

        double pred = lastLevel + h * lastTrend + seasonVal;
        predictions.append(pred);
    }

    const_cast<HoltWinters*>(this)->m_stats.totalForecasts += steps;
    const_cast<HoltWinters*>(this)->emit forecastCompleted(steps);
    return predictions;
}

HoltWinters::FitResult HoltWinters::autoFit(
    const QVector<double>& data, int seasonLength)
{
    FitResult bestResult;
    double bestMape = 1e30;

    /* 粗粒度网格搜索 */
    for (double a = 0.1; a <= 0.9; a += 0.2) {
        for (double b = 0.0; b <= 0.5; b += 0.1) {
            for (double g = 0.0; g <= 0.5; g += 0.1) {
                FitResult candidate;
                candidate.alpha = a;
                candidate.beta = b;
                candidate.gamma = g;

                /* 手动拟合计算 */
                int n = data.size();
                if (n < 2 * seasonLength) continue;

                QVector<double> level(n), fitted(n);
                level[0] = data[0];
                double trend = (data[seasonLength] - data[0]) / seasonLength;

                for (int i = 1; i < n; ++i) {
                    level[i] = a * data[i] + (1.0 - a) * (level[i - 1] + trend);
                    trend = b * (level[i] - level[i - 1]) + (1.0 - b) * trend;
                    fitted[i] = level[i - 1] + trend;
                }

                double mape = computeMape(data, fitted);
                if (mape < bestMape) {
                    bestMape = mape;
                    bestResult = candidate;
                }
            }
        }
    }

    bestResult.mape = bestMape;
    m_stats.totalFits++;
    emit fitCompleted(bestMape);
    return bestResult;
}

double HoltWinters::computeMape(const QVector<double>& actual,
                                 const QVector<double>& fitted) const
{
    int n = qMin(actual.size(), fitted.size());
    if (n == 0) return 0.0;

    double sum = 0.0;
    int count = 0;
    for (int i = 1; i < n; ++i) {
        if (qAbs(actual[i]) > 1e-10) {
            sum += qAbs((actual[i] - fitted[i]) / actual[i]);
            count++;
        }
    }
    return (count > 0) ? 100.0 * sum / count : 0.0;
}

void HoltWinters::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file BoxCounting.cpp
 * @brief 盒计数法分形维数实现
 */

#include "BoxCounting.h"
#include <QElapsedTimer>
#include <QSet>
#include <cmath>

BoxCounting::BoxCounting(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

int BoxCounting::countBoxes(const QVector<QPair<double, double>>& points,
                             double boxSize) const
{
    QSet<QString> occupied;
    for (const auto& p : points) {
        int bx = static_cast<int>(std::floor(p.first / boxSize));
        int by = static_cast<int>(std::floor(p.second / boxSize));
        occupied.insert(QString::number(bx) + QLatin1Char(',') + QString::number(by));
    }
    return occupied.size();
}

QPair<double, double> BoxCounting::linearRegression(
    const QVector<QPair<double, double>>& data) const
{
    int n = data.size();
    if (n < 2) return {0.0, 0.0};

    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (const auto& p : data) {
        sumX += p.first;
        sumY += p.second;
        sumXY += p.first * p.second;
        sumX2 += p.first * p.first;
    }

    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    double intercept = (sumY - slope * sumX) / n;

    /* R² */
    double yMean = sumY / n;
    double ssTot = 0, ssRes = 0;
    for (const auto& p : data) {
        double pred = slope * p.first + intercept;
        ssTot += (p.second - yMean) * (p.second - yMean);
        ssRes += (p.second - pred) * (p.second - pred);
    }
    double rSquared = (ssTot > 1e-15) ? 1.0 - ssRes / ssTot : 1.0;

    return {slope, rSquared};
}

QPair<double, double> BoxCounting::computeDimension(
    const QVector<QPair<double, double>>& points,
    double minBoxSize, double maxBoxSize, int steps) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> logData;
    double logMin = std::log(minBoxSize);
    double logMax = std::log(maxBoxSize);
    double step = (steps > 1) ? (logMax - logMin) / (steps - 1) : 0;

    for (int i = 0; i < steps; ++i) {
        double scale = std::exp(logMin + i * step);
        int count = countBoxes(points, scale);
        if (count > 0) {
            logData.append({std::log(1.0 / scale), std::log(static_cast<double>(count))});
        }
    }

    auto result = linearRegression(logData);

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(result.first, result.second);
    return result;
}

QPair<double, double> BoxCounting::computeImageDimension(
    const QVector<QVector<double>>& image,
    double threshold, int steps) const
{
    QElapsedTimer timer;
    timer.start();

    /* 转为前景像素点集 */
    QVector<QPair<double, double>> points;
    for (int y = 0; y < image.size(); ++y) {
        for (int x = 0; x < image[y].size(); ++x) {
            if (image[y][x] >= threshold)
                points.append({static_cast<double>(x), static_cast<double>(y)});
        }
    }

    int maxDim = 0;
    for (const auto& row : image)
        maxDim = qMax(maxDim, row.size());
    maxDim = qMax(maxDim, image.size());

    double minBox = 1.0;
    double maxBox = static_cast<double>(maxDim);

    auto result = computeDimension(points, minBox, maxBox, steps);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalComputations);

    return result;
}

QVector<QPair<double, double>> BoxCounting::boxCounts(
    const QVector<QPair<double, double>>& points,
    const QVector<double>& scales) const
{
    QVector<QPair<double, double>> result;
    for (double s : scales) {
        int count = countBoxes(points, s);
        if (count > 0)
            result.append({std::log(1.0 / s), std::log(static_cast<double>(count))});
    }
    return result;
}

BoxCounting::Stats BoxCounting::stats() const { return m_stats; }

void BoxCounting::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

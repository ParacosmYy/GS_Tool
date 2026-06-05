/**
 * @file ZScoreNormalizer.cpp
 * @brief Z-Score标准化器实现
 */

#include "utils/zscore/ZScoreNormalizer.h"
#include <QElapsedTimer>
#include <QtMath>

ZScoreNormalizer::ZScoreNormalizer(QObject* parent)
    : QObject(parent), m_outlierThreshold(3.0),
      m_sum(0.0), m_sqSum(0.0), m_count(0), m_timeSum(0.0) {}

void ZScoreNormalizer::setOutlierThreshold(double z) { m_outlierThreshold = z; }

QVector<double> ZScoreNormalizer::normalize(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (data.isEmpty()) return result;

    /* 计算均值和标准差 */
    double sum = 0.0;
    for (double v : data) sum += v;
    double mean = sum / data.size();

    double sqSum = 0.0;
    for (double v : data) { double d = v - mean; sqSum += d * d; }
    double stddev = qSqrt(sqSum / data.size());

    result.resize(data.size());
    if (stddev < 1e-15) {
        result.fill(0.0);
    } else {
        for (int i = 0; i < data.size(); ++i) {
            result[i] = (data[i] - mean) / stddev;
            if (qAbs(result[i]) > m_outlierThreshold) {
                ++m_stats.totalOutliers;
                emit outlierDetected(i, data[i], result[i]);
            }
        }
    }

    m_sum = sum;
    m_sqSum = sqSum;
    m_count = data.size();
    m_stats.currentMean = mean;
    m_stats.currentStddev = stddev;

    quint64 total = m_stats.totalNormalized + data.size();
    m_stats.totalNormalized = total;
    m_stats.outlierRate = (total > 0) ? static_cast<double>(m_stats.totalOutliers) / total : 0.0;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum;

    emit statsUpdated(mean, stddev);
    return result;
}

double ZScoreNormalizer::normalizeSingle(double value)
{
    updateStats(value);
    double sd = stddev();
    double z = (sd > 1e-15) ? (value - mean()) / sd : 0.0;

    ++m_stats.totalNormalized;
    if (qAbs(z) > m_outlierThreshold) {
        ++m_stats.totalOutliers;
        emit outlierDetected(m_count - 1, value, z);
    }
    m_stats.outlierRate = (m_stats.totalNormalized > 0)
        ? static_cast<double>(m_stats.totalOutliers) / m_stats.totalNormalized : 0.0;

    return z;
}

double ZScoreNormalizer::denormalize(double zScore) const
{
    return zScore * stddev() + mean();
}

double ZScoreNormalizer::mean() const
{
    return (m_count > 0) ? m_sum / m_count : 0.0;
}

double ZScoreNormalizer::stddev() const
{
    if (m_count < 2) return 0.0;
    double m = mean();
    double variance = (m_sqSum / m_count) - m * m;
    return qSqrt(qMax(0.0, variance));
}

int ZScoreNormalizer::count() const { return m_count; }

void ZScoreNormalizer::updateStats(double value)
{
    m_sum += value;
    m_sqSum += value * value;
    ++m_count;
    m_stats.currentMean = mean();
    m_stats.currentStddev = stddev();
}

void ZScoreNormalizer::reset()
{
    m_sum = 0.0;
    m_sqSum = 0.0;
    m_count = 0;
}

void ZScoreNormalizer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; reset(); }

/**
 * @file OutlierDetector.cpp
 * @brief 离群值检测器实现
 */

#include "utils/outlier/OutlierDetector.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

OutlierDetector::OutlierDetector(QObject* parent)
    : QObject(parent), m_method(Method::ZScore),
      m_threshold(3.0), m_epsilon(1.0), m_minSamples(3),
      m_timeSum(0.0) {}

void OutlierDetector::setMethod(Method m) { m_method = m; }
void OutlierDetector::setThreshold(double t) { m_threshold = t; }
void OutlierDetector::setEpsilon(double eps) { m_epsilon = qMax(1e-10, eps); }
void OutlierDetector::setMinSamples(int pts) { m_minSamples = qMax(1, pts); }

QList<OutlierDetector::Outlier> OutlierDetector::detect(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    QList<Outlier> result;
    if (data.size() < 2) return result;

    switch (m_method) {
    case Method::ZScore:   result = zScore(data); break;
    case Method::IQR:      result = iqrMethod(data); break;
    case Method::MAD:      result = madMethod(data); break;
    case Method::Grubbs:   result = grubbsMethod(data); break;
    case Method::DBSCAN:   result = dbscanMethod(data); break;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalDetections;
    m_stats.totalOutliersFound += result.size();
    m_stats.totalPointsProcessed += data.size();
    m_stats.outlierRate = (m_stats.totalPointsProcessed > 0)
        ? static_cast<double>(m_stats.totalOutliersFound) / m_stats.totalPointsProcessed : 0.0;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionComplete(result.size(), data.size());
    return result;
}

QList<OutlierDetector::Outlier> OutlierDetector::zScore(const QVector<double>& data)
{
    QList<Outlier> outliers;
    double sum = 0.0;
    for (double v : data) sum += v;
    double mean = sum / data.size();

    double sqSum = 0.0;
    for (double v : data) { double d = v - mean; sqSum += d * d; }
    double stddev = qSqrt(sqSum / data.size());
    if (stddev < 1e-10) return outliers;

    for (int i = 0; i < data.size(); ++i) {
        double z = qAbs(data[i] - mean) / stddev;
        if (z > m_threshold) {
            Outlier o{i, data[i], z};
            outliers.append(o);
            emit outlierFound(o);
        }
    }
    return outliers;
}

QList<OutlierDetector::Outlier> OutlierDetector::iqrMethod(const QVector<double>& data)
{
    QList<Outlier> outliers;
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double q1 = sorted[n / 4];
    double q3 = sorted[3 * n / 4];
    double iqr = q3 - q1;
    double lower = q1 - 1.5 * iqr;
    double upper = q3 + 1.5 * iqr;

    for (int i = 0; i < data.size(); ++i) {
        if (data[i] < lower || data[i] > upper) {
            double dist = qMax(qAbs(data[i] - lower), qAbs(data[i] - upper));
            Outlier o{i, data[i], dist / (iqr > 0 ? iqr : 1.0)};
            outliers.append(o);
            emit outlierFound(o);
        }
    }
    return outliers;
}

QList<OutlierDetector::Outlier> OutlierDetector::madMethod(const QVector<double>& data)
{
    QList<Outlier> outliers;
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double median = (n % 2 == 0) ? (sorted[n/2-1] + sorted[n/2]) / 2.0 : sorted[n/2];

    QVector<double> absDevs;
    absDevs.reserve(n);
    for (double v : data) absDevs.append(qAbs(v - median));
    std::sort(absDevs.begin(), absDevs.end());
    double mad = (n % 2 == 0) ? (absDevs[n/2-1] + absDevs[n/2]) / 2.0 : absDevs[n/2];
    if (mad < 1e-10) return outliers;

    /* MAD → σ 乘数: 0.6745 为正态分布下的常数 */
    double scale = 0.6745;
    for (int i = 0; i < data.size(); ++i) {
        double modifiedZ = qAbs(data[i] - median) / (scale * mad);
        if (modifiedZ > m_threshold) {
            Outlier o{i, data[i], modifiedZ};
            outliers.append(o);
            emit outlierFound(o);
        }
    }
    return outliers;
}

QList<OutlierDetector::Outlier> OutlierDetector::grubbsMethod(const QVector<double>& data)
{
    QList<Outlier> outliers;
    if (data.size() < 3) return outliers;

    QVector<double> remaining = data;
    QList<int> originalIndices;
    for (int i = 0; i < data.size(); ++i) originalIndices.append(i);

    while (remaining.size() >= 3) {
        double sum = 0.0;
        for (double v : remaining) sum += v;
        double mean = sum / remaining.size();

        double sqSum = 0.0;
        for (double v : remaining) { double d = v - mean; sqSum += d * d; }
        double stddev = qSqrt(sqSum / remaining.size());
        if (stddev < 1e-10) break;

        int maxIdx = 0;
        double maxDev = 0.0;
        for (int i = 0; i < remaining.size(); ++i) {
            double dev = qAbs(remaining[i] - mean);
            if (dev > maxDev) { maxDev = dev; maxIdx = i; }
        }

        double g = maxDev / stddev;
        /* Grubbs临界值近似 (α=0.05) */
        int n = remaining.size();
        double tcrit = 1.96 + (2.4 / qSqrt(n));
        double gcrit = ((n - 1) * tcrit) / qSqrt(n * (n - 2 + tcrit * tcrit));

        if (g > gcrit) {
            Outlier o{originalIndices[maxIdx], remaining[maxIdx], g};
            outliers.append(o);
            emit outlierFound(o);
            remaining.removeAt(maxIdx);
            originalIndices.removeAt(maxIdx);
        } else {
            break;
        }
    }
    return outliers;
}

QList<OutlierDetector::Outlier> OutlierDetector::dbscanMethod(const QVector<double>& data)
{
    QList<Outlier> outliers;
    int n = data.size();
    QVector<int> labels(n, -1); ///< -1 = 未分类, -2 = 噪声
    int clusterId = 0;

    for (int i = 0; i < n; ++i) {
        if (labels[i] != -1) continue;

        QVector<int> neighbors;
        for (int j = 0; j < n; ++j) {
            if (qAbs(data[i] - data[j]) <= m_epsilon) neighbors.append(j);
        }

        if (neighbors.size() < m_minSamples) {
            labels[i] = -2; // 噪声
            continue;
        }

        labels[i] = clusterId;
        for (int k = 0; k < neighbors.size(); ++k) {
            int j = neighbors[k];
            if (labels[j] == -2) labels[j] = clusterId;
            if (labels[j] != -1) continue;
            labels[j] = clusterId;

            QVector<int> jNeighbors;
            for (int m = 0; m < n; ++m) {
                if (qAbs(data[j] - data[m]) <= m_epsilon) jNeighbors.append(m);
            }
            if (jNeighbors.size() >= m_minSamples) {
                for (int nn : jNeighbors) {
                    if (!neighbors.contains(nn)) neighbors.append(nn);
                }
            }
        }
        ++clusterId;
    }

    for (int i = 0; i < n; ++i) {
        if (labels[i] == -2) {
            /* 计算到最近聚类中心的距离作为离群分数 */
            double minDist = std::numeric_limits<double>::max();
            for (int j = 0; j < n; ++j) {
                if (labels[j] >= 0) {
                    double d = qAbs(data[i] - data[j]);
                    if (d < minDist) minDist = d;
                }
            }
            Outlier o{i, data[i], minDist};
            outliers.append(o);
            emit outlierFound(o);
        }
    }
    return outliers;
}

void OutlierDetector::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }

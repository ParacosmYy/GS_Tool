/**
 * @file OutlierDetector.cpp
 * @brief 异常值检测引擎实现 — Z-Score/IQR/MAD/滚动窗口
 */

#include "utils/outlier/OutlierDetector.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
OutlierDetector::OutlierDetector(QObject* parent)
    : QObject(parent)
    , m_method(DetectionMethod::ZScore)
    , m_zThreshold(3.0)
    , m_iqrMultiplier(1.5)
    , m_madThreshold(3.0)
    , m_windowSize(50)
    , m_rollingThreshold(2.0)
    , m_scoreSum(0.0)
{
}

/** @brief 设置检测方法 @param method 方法 */
void OutlierDetector::setMethod(DetectionMethod method)
{
    m_method = method;
}

/** @brief 设置Z-Score阈值 @param threshold N倍标准差 */
void OutlierDetector::setZScoreThreshold(double threshold)
{
    m_zThreshold = qMax(1.0, threshold);
}

/** @brief 设置IQR倍数 @param multiplier 倍数 */
void OutlierDetector::setIqrMultiplier(double multiplier)
{
    m_iqrMultiplier = qMax(0.5, multiplier);
}

/** @brief 设置MAD阈值 @param threshold N倍MAD */
void OutlierDetector::setMadThreshold(double threshold)
{
    m_madThreshold = qMax(1.0, threshold);
}

/** @brief 设置滚动窗口大小 @param size 窗口点数 */
void OutlierDetector::setWindowSize(int size)
{
    m_windowSize = qMax(3, size);
}

/** @brief 设置滚动窗口偏差阈值 @param threshold 阈值 */
void OutlierDetector::setRollingThreshold(double threshold)
{
    m_rollingThreshold = qMax(0.1, threshold);
}

/** @brief 检测异常值 @param data 数据 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::detect(
    const QVector<double>& data)
{
    if (data.size() < 3) {
        return {};
    }

    QList<Outlier> outliers;
    switch (m_method) {
    case DetectionMethod::ZScore:
        outliers = detectZScore(data);
        break;
    case DetectionMethod::IQR:
        outliers = detectIQR(data);
        break;
    case DetectionMethod::MAD:
        outliers = detectMAD(data);
        break;
    case DetectionMethod::RollingWindow:
        outliers = detectRolling(data);
        break;
    }

    /* 更新统计 */
    m_stats.totalPointsChecked += static_cast<quint64>(data.size());
    m_stats.totalOutliersFound += static_cast<quint64>(outliers.size());
    if (m_stats.totalPointsChecked > 0) {
        m_stats.outlierRate = static_cast<double>(m_stats.totalOutliersFound)
            / static_cast<double>(m_stats.totalPointsChecked);
    }

    for (const auto& o : outliers) {
        double dev = qAbs(o.value - o.expected);
        if (dev > m_stats.peakDeviation) {
            m_stats.peakDeviation = dev;
        }
        m_scoreSum += o.score;
    }
    if (m_stats.totalOutliersFound > 0) {
        m_stats.averageScore = m_scoreSum
            / static_cast<double>(m_stats.totalOutliersFound);
    }

    m_lastOutliers = outliers;
    emit detectionComplete(outliers);
    return outliers;
}

/** @brief 实时检测单个值 @param value 新值 @return 异常点 */
OutlierDetector::Outlier OutlierDetector::detectPoint(double value)
{
    m_streamBuffer.append(value);
    if (m_streamBuffer.size() > m_windowSize * 2) {
        m_streamBuffer.remove(0, m_streamBuffer.size() - m_windowSize * 2);
    }

    Outlier result;
    result.index = static_cast<int>(m_stats.totalPointsChecked);
    result.value = value;
    result.score = 0.0;
    result.expected = 0.0;

    if (m_streamBuffer.size() < 3) {
        ++m_stats.totalPointsChecked;
        return result;
    }

    /* 计算均值和标准差 */
    double sum = 0.0;
    for (double v : m_streamBuffer) {
        sum += v;
    }
    double mean = sum / m_streamBuffer.size();

    double sqSum = 0.0;
    for (double v : m_streamBuffer) {
        double d = v - mean;
        sqSum += d * d;
    }
    double stddev = qSqrt(sqSum / m_streamBuffer.size());

    result.expected = mean;

    if (stddev > 1e-10) {
        double zScore = qAbs(value - mean) / stddev;
        result.score = zScore;
        if (zScore > m_zThreshold) {
            ++m_stats.totalPointsChecked;
            ++m_stats.totalOutliersFound;
            m_stats.outlierRate = static_cast<double>(m_stats.totalOutliersFound)
                / static_cast<double>(m_stats.totalPointsChecked);
            m_scoreSum += result.score;
            m_stats.averageScore = m_scoreSum
                / static_cast<double>(m_stats.totalOutliersFound);
            double dev = qAbs(value - mean);
            if (dev > m_stats.peakDeviation) {
                m_stats.peakDeviation = dev;
            }
            emit outlierDetected(result);
            return result;
        }
    }

    ++m_stats.totalPointsChecked;
    result.score = 0.0;
    return result;
}

/** @brief 获取最近检测结果 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::lastOutliers() const
{
    return m_lastOutliers;
}

/** @brief 重置统计 */
void OutlierDetector::resetStatistics()
{
    m_stats = Stats{};
    m_scoreSum = 0.0;
    m_streamBuffer.clear();
}

/** @brief Z-Score检测 @param data 数据 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::detectZScore(
    const QVector<double>& data)
{
    QList<Outlier> outliers;

    double sum = 0.0;
    for (double v : data) {
        sum += v;
    }
    double mean = sum / data.size();

    double sqSum = 0.0;
    for (double v : data) {
        double d = v - mean;
        sqSum += d * d;
    }
    double stddev = qSqrt(sqSum / data.size());

    if (stddev < 1e-10) {
        return outliers;
    }

    for (int i = 0; i < data.size(); ++i) {
        double zScore = qAbs(data[i] - mean) / stddev;
        if (zScore > m_zThreshold) {
            Outlier o;
            o.index = i;
            o.value = data[i];
            o.score = zScore;
            o.expected = mean;
            outliers.append(o);
            emit outlierDetected(o);
        }
    }

    return outliers;
}

/** @brief IQR检测 @param data 数据 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::detectIQR(
    const QVector<double>& data)
{
    QList<Outlier> outliers;
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    double q1 = sorted[n / 4];
    double q3 = sorted[3 * n / 4];
    double iqr = q3 - q1;
    double lowerBound = q1 - m_iqrMultiplier * iqr;
    double upperBound = q3 + m_iqrMultiplier * iqr;
    double median = sorted[n / 2];

    for (int i = 0; i < data.size(); ++i) {
        if (data[i] < lowerBound || data[i] > upperBound) {
            Outlier o;
            o.index = i;
            o.value = data[i];
            o.expected = median;
            double dist = qMin(qAbs(data[i] - lowerBound), qAbs(data[i] - upperBound));
            o.score = (iqr > 1e-10) ? dist / iqr : 0.0;
            outliers.append(o);
            emit outlierDetected(o);
        }
    }

    return outliers;
}

/** @brief MAD检测 @param data 数据 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::detectMAD(
    const QVector<double>& data)
{
    QList<Outlier> outliers;

    /* 计算中位数 */
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    double median = (n % 2 == 0)
        ? (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0
        : sorted[n / 2];

    /* 计算MAD */
    QVector<double> deviations;
    deviations.reserve(n);
    for (double v : data) {
        deviations.append(qAbs(v - median));
    }
    std::sort(deviations.begin(), deviations.end());
    double mad = (n % 2 == 0)
        ? (deviations[n / 2 - 1] + deviations[n / 2]) / 2.0
        : deviations[n / 2];

    if (mad < 1e-10) {
        return outliers;
    }

    /* 0.6745是正态分布的MAD比例因子 */
    for (int i = 0; i < data.size(); ++i) {
        double modifiedZ = 0.6745 * (data[i] - median) / mad;
        if (qAbs(modifiedZ) > m_madThreshold) {
            Outlier o;
            o.index = i;
            o.value = data[i];
            o.expected = median;
            o.score = qAbs(modifiedZ);
            outliers.append(o);
            emit outlierDetected(o);
        }
    }

    return outliers;
}

/** @brief 滚动窗口检测 @param data 数据 @return 异常点列表 */
QList<OutlierDetector::Outlier> OutlierDetector::detectRolling(
    const QVector<double>& data)
{
    QList<Outlier> outliers;

    for (int i = 0; i < data.size(); ++i) {
        int start = qMax(0, i - m_windowSize / 2);
        int end = qMin(data.size() - 1, i + m_windowSize / 2);
        int count = end - start + 1;

        double sum = 0.0;
        for (int j = start; j <= end; ++j) {
            sum += data[j];
        }
        double mean = sum / count;

        double sqSum = 0.0;
        for (int j = start; j <= end; ++j) {
            double d = data[j] - mean;
            sqSum += d * d;
        }
        double stddev = qSqrt(sqSum / count);

        double deviation = (stddev > 1e-10)
            ? qAbs(data[i] - mean) / stddev : 0.0;

        if (deviation > m_rollingThreshold) {
            Outlier o;
            o.index = i;
            o.value = data[i];
            o.expected = mean;
            o.score = deviation;
            outliers.append(o);
            emit outlierDetected(o);
        }
    }

    return outliers;
}

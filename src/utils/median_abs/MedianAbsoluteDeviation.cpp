/**
 * @file MedianAbsoluteDeviation.cpp
 * @brief 中位数绝对偏差(MAD)异常值检测器实现
 */

#include "utils/median_abs/MedianAbsoluteDeviation.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
MedianAbsoluteDeviation::MedianAbsoluteDeviation(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算中位数(排序后取中间值)
 * @param data 输入数据
 * @return 中位数
 */
double MedianAbsoluteDeviation::computeMedian(const QVector<double>& data)
{
    if (data.isEmpty()) return 0.0;

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    if (n % 2 == 1) {
        return sorted[n / 2];
    }
    return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
}

/**
 * @brief 计算数据集的MAD值
 * @param data 输入数据
 * @return MAD值(中位数绝对偏差)
 *
 * MAD = median(|xi - median(x)|)，对异常值具有50%折断点，
 * 远比标准差鲁棒。
 */
double MedianAbsoluteDeviation::compute(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < 2) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalComputed;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;
        return 0.0;
    }

    /* 第一步: 计算中位数 */
    double median = computeMedian(data);

    /* 第二步: 计算绝对偏差 */
    QVector<double> absDevs;
    absDevs.reserve(data.size());
    for (double v : data) {
        absDevs.append(std::abs(v - median));
    }

    /* 第三步: 计算绝对偏差的中位数即MAD */
    double mad = computeMedian(absDevs);

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalComputed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;

    return mad;
}

/**
 * @brief 检测异常值索引
 * @param data 输入数据
 * @param threshold 鲁棒Z分数阈值(默认3.5)
 * @return 异常值在data中的索引列表
 *
 * 鲁棒Z分数 = 0.6745 * (xi - median) / MAD
 * 阈值3.5对应正态分布下约99.7%置信区间外的数据点。
 */
QVector<int> MedianAbsoluteDeviation::detectOutliers(
    const QVector<double>& data, double threshold)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> outliers;

    if (data.size() < 2) {
        emit outliersDetected(0);
        return outliers;
    }

    /* 计算中位数和MAD */
    double median = computeMedian(data);

    QVector<double> absDevs;
    absDevs.reserve(data.size());
    for (double v : data) {
        absDevs.append(std::abs(v - median));
    }
    double mad = computeMedian(absDevs);

    /* MAD为零时所有值相同，无异常值 */
    if (mad < 1e-15) {
        emit outliersDetected(0);
        return outliers;
    }

    /* 计算鲁棒Z分数并检测异常 */
    for (int i = 0; i < data.size(); ++i) {
        double zscore = robustZScore(data[i], median, mad);
        if (std::abs(zscore) > threshold) {
            outliers.append(i);
        }
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalComputed;
    m_stats.totalOutliers += static_cast<quint64>(outliers.size());
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalComputed;

    emit outliersDetected(outliers.size());
    return outliers;
}

/**
 * @brief 计算单个值的鲁棒Z分数
 * @param value 待计算的值
 * @param median 数据集中位数
 * @param mad 中位数绝对偏差
 * @return 鲁棒Z分数
 *
 * 0.6745是正态分布MAD与标准差的比率校正因子，
 * 使鲁棒Z分数与标准Z分数在正态分布下可比。
 */
double MedianAbsoluteDeviation::robustZScore(double value,
                                             double median,
                                             double mad) const
{
    if (mad < 1e-15) return 0.0;
    return 0.6745 * (value - median) / mad;
}

/** @brief 重置统计信息 */
void MedianAbsoluteDeviation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}

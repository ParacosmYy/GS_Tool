/**
 * @file LinearInterpolator.cpp
 * @brief 线性插值器实现 — 分段线性插值与反插值
 */

#include "utils/interp/LinearInterpolator.h"

#include <QtMath>
#include <algorithm>
#include <numeric>

/** @brief 构造函数 @param parent 父对象 */
LinearInterpolator::LinearInterpolator(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 设置控制点集合 @param points (x,y)点对列表 */
void LinearInterpolator::setPoints(const QVector<QPair<double, double>>& points)
{
    m_points = points;

    /* 按X坐标升序排序 */
    std::sort(m_points.begin(), m_points.end(),
              [](const QPair<double, double>& a, const QPair<double, double>& b) {
                  return a.first < b.first;
              });
}

/** @brief 在指定x处执行线性插值 @param x 目标X @return 插值Y值 */
double LinearInterpolator::interpolate(double x) const
{
    m_timer.start();

    double result = 0.0;

    if (m_points.isEmpty()) {
        result = 0.0;
    } else if (m_points.size() == 1) {
        result = m_points.first().second;
    } else {
        int seg = findSegment(x);

        /* 越界钳位到最近端点 */
        if (seg < 0) {
            result = m_points.first().second;
        } else if (seg >= m_points.size() - 1) {
            result = m_points.last().second;
        } else {
            double x0 = m_points[seg].first;
            double y0 = m_points[seg].second;
            double x1 = m_points[seg + 1].first;
            double y1 = m_points[seg + 1].second;
            double denom = x1 - x0;
            if (qFuzzyIsNull(denom)) {
                result = y0;
            } else {
                double t = (x - x0) / denom;
                result = y0 + t * (y1 - y0);
            }
        }
    }

    ++m_stats.totalInterpolations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    return result;
}

/** @brief 批量线性插值 @param xs 目标X列表 @return Y值列表 */
QVector<double> LinearInterpolator::interpolateBatch(
    const QVector<double>& xs) const
{
    m_timer.start();

    QVector<double> results;
    results.reserve(xs.size());

    for (double x : xs) {
        double y = 0.0;

        if (m_points.isEmpty()) {
            y = 0.0;
        } else if (m_points.size() == 1) {
            y = m_points.first().second;
        } else {
            int seg = findSegment(x);
            if (seg < 0) {
                y = m_points.first().second;
            } else if (seg >= m_points.size() - 1) {
                y = m_points.last().second;
            } else {
                double x0 = m_points[seg].first;
                double y0 = m_points[seg].second;
                double x1 = m_points[seg + 1].first;
                double y1 = m_points[seg + 1].second;
                double denom = x1 - x0;
                if (qFuzzyIsNull(denom)) {
                    y = y0;
                } else {
                    double t = (x - x0) / denom;
                    y = y0 + t * (y1 - y0);
                }
            }
        }
        results.append(y);
    }

    m_stats.totalInterpolations += static_cast<quint64>(xs.size());
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    emit interpolationComplete(xs.size());
    return results;
}

/** @brief 反插值 — 给定Y值求X @param y 目标Y值 @return 对应X值 */
double LinearInterpolator::inverseInterpolate(double y) const
{
    m_timer.start();

    double result = qQNaN();

    if (m_points.size() < 2) {
        /* 至少需要两个点才能反插值 */
    } else {
        /* 遍历每一段，查找Y值跨越目标值的区间 */
        for (int i = 0; i < m_points.size() - 1; ++i) {
            double y0 = m_points[i].second;
            double y1 = m_points[i + 1].second;

            /* 检查y是否在[y0, y1]或[y1, y0]之间 */
            bool crossing = (y0 <= y && y <= y1) || (y1 <= y && y <= y0);
            if (crossing) {
                double x0 = m_points[i].first;
                double x1 = m_points[i + 1].first;
                double denom = y1 - y0;
                if (qFuzzyIsNull(denom)) {
                    /* 水平段，返回段中点 */
                    result = (x0 + x1) / 2.0;
                } else {
                    double t = (y - y0) / denom;
                    result = x0 + t * (x1 - x0);
                }
                break; /* 返回第一个匹配 */
            }
        }
    }

    ++m_stats.totalInterpolations;
    m_timeSum += static_cast<double>(m_timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterpolations);

    return result;
}

/** @brief 重置统计 */
void LinearInterpolator::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

/** @brief 二分查找x所属的段索引 @param x 目标X @return 段索引 */
int LinearInterpolator::findSegment(double x) const
{
    if (m_points.isEmpty()) return -1;

    int n = m_points.size();
    if (x <= m_points.first().first) return 0;
    if (x >= m_points.last().first)  return n - 1;

    int lo = 0, hi = n - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (m_points[mid].first <= x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

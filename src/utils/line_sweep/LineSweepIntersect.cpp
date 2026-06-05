/**
 * @file LineSweepIntersect.cpp
 * @brief 线段交点检测实现 — 扫描线算法
 */

#include "utils/line_sweep/LineSweepIntersect.h"

#include <QElapsedTimer>
#include <algorithm>
#include <set>
#include <cmath>

// ============================================================================
// 构造
// ============================================================================

LineSweepIntersect::LineSweepIntersect(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

QVector<QPair<double,double>> LineSweepIntersect::findIntersections(
    QVector<QPair<QPair<double,double>,QPair<double,double>>> segments)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double,double>> intersections;

    int n = segments.size();
    if (n < 2) {
        ++m_stats.totalComputations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computationCompleted(0);
        return intersections;
    }

    /* 事件点类型: 上端点/下端点/交点 */
    enum EventType { UPPER, LOWER, CROSS };

    struct Event {
        double x, y;          ///< 事件点坐标
        EventType type;       ///< 类型
        int segIdx;           ///< 线段索引
        int otherIdx;         ///< 交点事件的另一条线段
    };

    /* 规范化线段: 确保上端点y坐标更大(或x更小) */
    for (int i = 0; i < n; ++i) {
        auto& s = segments[i];
        auto& p1 = s.first;
        auto& p2 = s.second;
        if (p1.second < p2.second ||
            (p1.second == p2.second && p1.first > p2.first)) {
            std::swap(p1, p2);
        }
    }

    /* 构建初始事件队列(所有线段的上端点和下端点) */
    QVector<Event> events;
    events.reserve(n * 2);

    for (int i = 0; i < n; ++i) {
        events.append({segments[i].first.first,
                       segments[i].first.second, UPPER, i, -1});
        events.append({segments[i].second.first,
                       segments[i].second.second, LOWER, i, -1});
    }

    /* 按y降序排列事件(扫描线从上到下) */
    auto eventCmp = [](const Event& a, const Event& b) {
        if (std::abs(a.y - b.y) > 1e-12) return a.y > b.y;
        if (std::abs(a.x - b.x) > 1e-12) return a.x < b.x;
        return a.type < b.type;
    };

    std::sort(events.begin(), events.end(), eventCmp);

    /* 活跃线段集合(按当前扫描线位置的x坐标排序) */
    std::set<int> active;
    QVector<QPair<double,double>> results;

    /* 简化的扫描线: 对活跃线段逐一检查交点 */
    for (int ei = 0; ei < events.size(); ++ei) {
        const Event& ev = events[ei];

        if (ev.type == UPPER) {
            /* 线段上端点: 插入活跃集 */
            active.insert(ev.segIdx);

            /* 检查与相邻活跃线段的交点 */
            for (int aj : active) {
                if (aj == ev.segIdx) continue;
                QPair<double,double> ip;
                if (segmentIntersect(segments[ev.segIdx].first,
                                     segments[ev.segIdx].second,
                                     segments[aj].first,
                                     segments[aj].second, ip)) {
                    /* 避免重复交点 */
                    bool dup = false;
                    for (const auto& existing : results) {
                        double dx = existing.first - ip.first;
                        double dy = existing.second - ip.second;
                        if (dx * dx + dy * dy < 1e-16) {
                            dup = true;
                            break;
                        }
                    }
                    if (!dup) {
                        results.append(ip);
                    }
                }
            }
        } else if (ev.type == LOWER) {
            /* 线段下端点: 从活跃集移除 */
            active.erase(ev.segIdx);
        }
    }

    intersections = results;

    ++m_stats.totalComputations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(intersections.size());
    return intersections;
}

void LineSweepIntersect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

bool LineSweepIntersect::segmentIntersect(
    const QPair<double,double>& a1, const QPair<double,double>& a2,
    const QPair<double,double>& b1, const QPair<double,double>& b2,
    QPair<double,double>& intersection)
{
    double d1x = a2.first - a1.first;
    double d1y = a2.second - a1.second;
    double d2x = b2.first - b1.first;
    double d2y = b2.second - b1.second;

    double denom = d1x * d2y - d1y * d2x;

    /* 平行或共线 */
    if (std::abs(denom) < 1e-14) return false;

    double dx = b1.first - a1.first;
    double dy = b1.second - a1.second;

    double t = (dx * d2y - dy * d2x) / denom;
    double u = (dx * d1y - dy * d1x) / denom;

    /* 检查参数是否在[0,1]范围内 */
    if (t < -1e-12 || t > 1.0 + 1e-12 ||
        u < -1e-12 || u > 1.0 + 1e-12) {
        return false;
    }

    /* 钳制到[0,1] */
    t = std::max(0.0, std::min(1.0, t));
    u = std::max(0.0, std::min(1.0, u));

    intersection.first = a1.first + t * d1x;
    intersection.second = a1.second + t * d1y;

    return true;
}

/**
 * @file LineSegmentIntersect.cpp
 * @brief 线段交集检测实现
 */

#include "utils/line_intersect/LineSegmentIntersect.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

LineSegmentIntersect::LineSegmentIntersect(QObject* parent)
    : QObject(parent) {}

QVector<LineSegmentIntersect::Point>
LineSegmentIntersect::findIntersections(const QVector<Segment>& segments)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Point> result;
    int n = segments.size();
    if (n < 2) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            (m_stats.totalChecks > 0) ? m_timeSum / m_stats.totalChecks : 0.0;
        return result;
    }

    if (n <= 64) {
        result = bruteForce(segments);
    } else {
        result = sweepLine(segments);
    }

    m_stats.totalIntersections += static_cast<quint64>(result.size());

    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(static_cast<quint64>(1), m_stats.totalChecks);

    emit searchCompleted(result.size());
    return result;
}

bool LineSegmentIntersect::intersects(const Segment& a, const Segment& b) const
{
    double d1 = cross(b.p1, b.p2, a.p1);
    double d2 = cross(b.p1, b.p2, a.p2);
    double d3 = cross(a.p1, a.p2, b.p1);
    double d4 = cross(a.p1, a.p2, b.p2);

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
        return true;
    }

    /* 共线退化检查 */
    auto onSegment = [](const Point& p, const Point& q, const Point& r) {
        return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
               q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
    };

    if (std::abs(d1) < 1e-12 && onSegment(b.p1, a.p1, b.p2)) return true;
    if (std::abs(d2) < 1e-12 && onSegment(b.p1, a.p2, b.p2)) return true;
    if (std::abs(d3) < 1e-12 && onSegment(a.p1, b.p1, a.p2)) return true;
    if (std::abs(d4) < 1e-12 && onSegment(a.p1, b.p2, a.p2)) return true;

    return false;
}

double LineSegmentIntersect::cross(const Point& p1, const Point& p2,
                                   const Point& p3) const
{
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

QVector<LineSegmentIntersect::Point>
LineSegmentIntersect::bruteForce(const QVector<Segment>& segments) const
{
    QVector<Point> result;
    int n = segments.size();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            m_stats.totalChecks++;
            if (intersects(segments[i], segments[j])) {
                result.append(computeIntersection(segments[i], segments[j]));
            }
        }
    }
    return result;
}

QVector<LineSegmentIntersect::Point>
LineSegmentIntersect::sweepLine(const QVector<Segment>& segments) const
{
    /*
     * 简化版扫描线：按线段最小x排序，维护活跃线段集，
     * 仅检测y范围可能重叠的活跃线段对。
     */
    struct Event {
        double x;
        int    idx;
        bool   isStart;
    };

    int n = segments.size();
    QVector<Event> events;
    events.reserve(n * 2);

    for (int i = 0; i < n; ++i) {
        double minX = std::min(segments[i].p1.x, segments[i].p2.x);
        double maxX = std::max(segments[i].p1.x, segments[i].p2.x);
        events.append({minX, i, true});
        events.append({maxX, i, false});
    }

    std::sort(events.begin(), events.end(),
              [](const Event& a, const Event& b) {
                  if (a.x != b.x) return a.x < b.x;
                  return a.isStart > b.isStart;
              });

    QVector<Point> result;
    std::vector<int> active;

    for (const auto& ev : events) {
        if (ev.isStart) {
            /* 与所有活跃线段检测交点 */
            for (int j : active) {
                m_stats.totalChecks++;
                if (intersects(segments[ev.idx], segments[j])) {
                    result.append(
                        computeIntersection(segments[ev.idx], segments[j]));
                }
            }
            active.push_back(ev.idx);
        } else {
            /* 从活跃集中移除 */
            auto it = std::find(active.begin(), active.end(), ev.idx);
            if (it != active.end()) active.erase(it);
        }
    }
    return result;
}

LineSegmentIntersect::Point
LineSegmentIntersect::computeIntersection(const Segment& a,
                                          const Segment& b) const
{
    double a1 = a.p2.y - a.p1.y;
    double a2 = a.p2.x - a.p1.x;
    double b1 = b.p2.y - b.p1.y;
    double b2 = b.p2.x - b.p1.x;

    double denom = a1 * b2 - a2 * b1;
    if (std::abs(denom) < 1e-15) {
        return {(a.p1.x + a.p2.x) / 2.0, (a.p1.y + a.p2.y) / 2.0};
    }

    double t = ((b.p1.x - a.p1.x) * b1 - (b.p1.y - a.p1.y) * b2) / denom;
    Point pt;
    pt.x = a.p1.x + t * a2;
    pt.y = a.p1.y + t * a1;
    return pt;
}

void LineSegmentIntersect::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}

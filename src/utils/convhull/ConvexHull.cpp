/**
 * @file ConvexHull.cpp
 * @brief 凸包算法实现 — Andrew's Monotone Chain
 */

#include "utils/convhull/ConvexHull.h"

#include <QElapsedTimer>
#include <algorithm>

ConvexHull::ConvexHull(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double ConvexHull::cross(const Point& o, const Point& a, const Point& b)
{
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

double ConvexHull::shoelaceArea(const QVector<Point>& hull)
{
    if (hull.size() < 3) return 0.0;
    double area = 0.0;
    int n = hull.size();
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += hull[i].x * hull[j].y;
        area -= hull[j].x * hull[i].y;
    }
    return qAbs(area) / 2.0;
}

QVector<ConvexHull::Point> ConvexHull::compute(const QVector<Point>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Point> pts = points;
    int n = pts.size();
    if (n < 3) { m_hull = pts; return m_hull; }

    std::sort(pts.begin(), pts.end(),
              [](const Point& a, const Point& b) {
                  return (a.x < b.x) || (a.x == b.x && a.y < b.y);
              });

    /* 构建下凸包 */
    QVector<Point> hull;
    for (int i = 0; i < n; ++i) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size()-2], hull[hull.size()-1], pts[i]) <= 0)
            hull.removeLast();
        hull.append(pts[i]);
    }

    /* 构建上凸包 */
    int lowerSize = hull.size() + 1;
    for (int i = n - 2; i >= 0; --i) {
        while (hull.size() >= lowerSize &&
               cross(hull[hull.size()-2], hull[hull.size()-1], pts[i]) <= 0)
            hull.removeLast();
        hull.append(pts[i]);
    }

    hull.removeLast();
    m_hull = hull;

    double area = shoelaceArea(m_hull);
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit hullComputed(m_hull.size(), area);
    return m_hull;
}

bool ConvexHull::isInside(const Point& point)
{
    if (m_hull.size() < 3) return false;
    int n = m_hull.size();
    bool allPos = true, allNeg = true;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        double cp = (m_hull[j].x - m_hull[i].x) * (point.y - m_hull[i].y) -
                    (m_hull[j].y - m_hull[i].y) * (point.x - m_hull[i].x);
        if (cp < 0) allPos = false;
        if (cp > 0) allNeg = false;
    }
    return allPos || allNeg;
}

double ConvexHull::hullArea()
{
    return shoelaceArea(m_hull);
}

void ConvexHull::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

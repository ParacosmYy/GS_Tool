/**
 * @file PolygonClipper.cpp
 * @brief Sutherland-Hodgman多边形裁剪实现
 */

#include "PolygonClipper.h"
#include <QElapsedTimer>
#include <cmath>

PolygonClipper::PolygonClipper(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<PolygonClipper::Point> PolygonClipper::clipToRect(
    const QVector<Point>& polygon, double left, double right,
    double bottom, double top)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Point> output = polygon;

    /* 四条边依次裁剪 */
    Point bl = {left, bottom}, tl = {left, top};
    Point tr = {right, top}, br = {right, bottom};

    output = clipByEdge(output, bl, tl);  /* 左边 */
    output = clipByEdge(output, tl, tr);  /* 上边 */
    output = clipByEdge(output, tr, br);  /* 右边 */
    output = clipByEdge(output, br, bl);  /* 下边 */

    m_stats.totalClips++;
    m_stats.totalInputVertices += polygon.size();
    m_stats.totalOutputVertices += output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClips;

    emit clipped(polygon.size(), output.size());
    return output;
}

QVector<PolygonClipper::Point> PolygonClipper::clipToPolygon(
    const QVector<Point>& subject, const QVector<Point>& clipper)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Point> output = subject;
    int n = clipper.size();

    for (int i = 0; i < n && !output.isEmpty(); ++i) {
        output = clipByEdge(output, clipper[i], clipper[(i + 1) % n]);
    }

    m_stats.totalClips++;
    m_stats.totalInputVertices += subject.size();
    m_stats.totalOutputVertices += output.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClips;

    emit clipped(subject.size(), output.size());
    return output;
}

double PolygonClipper::polygonArea(const QVector<Point>& poly) const
{
    double area = 0.0;
    int n = poly.size();
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += poly[i].x * poly[j].y;
        area -= poly[j].x * poly[i].y;
    }
    return std::abs(area) / 2.0;
}

bool PolygonClipper::pointInPolygon(const Point& p,
                                      const QVector<Point>& poly) const
{
    int n = poly.size();
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y)
             / (poly[j].y - poly[i].y) + poly[i].x))
            inside = !inside;
    }
    return inside;
}

QVector<PolygonClipper::Point> PolygonClipper::clipByEdge(
    const QVector<Point>& poly, const Point& e1, const Point& e2)
{
    QVector<Point> output;
    int n = poly.size();
    if (n == 0) return output;

    for (int i = 0; i < n; ++i) {
        Point current = poly[i];
        Point next = poly[(i + 1) % n];

        bool curInside = isInside(current, e1, e2);
        bool nextInside = isInside(next, e1, e2);

        if (curInside) {
            output.append(current);
            if (!nextInside)
                output.append(intersect(current, next, e1, e2));
        } else if (nextInside) {
            output.append(intersect(current, next, e1, e2));
        }
    }

    return output;
}

PolygonClipper::Point PolygonClipper::intersect(const Point& p1,
    const Point& p2, const Point& e1, const Point& e2) const
{
    double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
    double dx2 = e2.x - e1.x, dy2 = e2.y - e1.y;
    double denom = dx1 * dy2 - dy1 * dx2;

    if (std::abs(denom) < 1e-15) return p1;

    double t = ((e1.x - p1.x) * dy2 - (e1.y - p1.y) * dx2) / denom;
    t = qBound(0.0, t, 1.0);

    return {p1.x + t * dx1, p1.y + t * dy1};
}

bool PolygonClipper::isInside(const Point& p, const Point& e1,
                                const Point& e2) const
{
    double cross = (e2.x - e1.x) * (p.y - e1.y)
                 - (e2.y - e1.y) * (p.x - e1.x);
    return cross >= 0;
}

PolygonClipper::Stats PolygonClipper::stats() const { return m_stats; }

void PolygonClipper::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

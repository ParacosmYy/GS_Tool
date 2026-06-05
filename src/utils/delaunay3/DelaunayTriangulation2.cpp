/**
 * @file DelaunayTriangulation2.cpp
 * @brief Delaunay三角剖分增强版实现
 */

#include "DelaunayTriangulation2.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

DelaunayTriangulation2::DelaunayTriangulation2(QObject* parent)
    : QObject(parent)
    , m_incrementalMode(false)
    , m_timeSum(0.0)
{
}

QVector<DelaunayTriangulation2::Triangle> DelaunayTriangulation2::triangulate(
    const QVector<Point2D>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Triangle> result;
    int n = points.size();
    if (n < 3) {
        m_stats.totalTriangulated++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTriangulated;
        return result;
    }

    /* 创建带超级三角形的点集 */
    m_points = points;

    double minX = points[0].x, minY = points[0].y;
    double maxX = minX, maxY = minY;
    for (const auto& p : points) {
        minX = qMin(minX, p.x); minY = qMin(minY, p.y);
        maxX = qMax(maxX, p.x); maxY = qMax(maxY, p.y);
    }

    double dx = maxX - minX, dy = maxY - minY;
    double dmax = qMax(dx, dy, 1.0);
    double midX = (minX + maxX) / 2.0, midY = (minY + maxY) / 2.0;

    /* 超级三角形顶点 */
    int p1 = n, p2 = n + 1, p3 = n + 2;
    m_points.append({midX - 20 * dmax, midY - dmax});
    m_points.append({midX, midY + 20 * dmax});
    m_points.append({midX + 20 * dmax, midY - dmax});

    m_triangles.clear();
    m_triangles.append({p1, p2, p3});

    /* Bowyer-Watson增量插入 */
    for (int i = 0; i < n; ++i) {
        QVector<Triangle> badTriangles;

        for (const auto& tri : m_triangles) {
            if (inCircumCircle(m_points[i],
                               m_points[tri.v0],
                               m_points[tri.v1],
                               m_points[tri.v2]))
                badTriangles.append(tri);
        }

        /* 找多边形边界 */
        QVector<Edge> polygon;
        for (const auto& tri : badTriangles) {
            Edge edges[3] = {{tri.v0, tri.v1}, {tri.v1, tri.v2}, {tri.v2, tri.v0}};
            for (const auto& edge : edges) {
                bool shared = false;
                for (const auto& other : badTriangles) {
                    if (&tri == &other) continue;
                    Edge oEdges[3] = {{other.v0, other.v1}, {other.v1, other.v2}, {other.v2, other.v0}};
                    for (const auto& oe : oEdges) {
                        if ((edge.v0 == oe.v0 && edge.v1 == oe.v1) ||
                            (edge.v0 == oe.v1 && edge.v1 == oe.v0)) {
                            shared = true; break;
                        }
                    }
                    if (shared) break;
                }
                if (!shared) polygon.append(edge);
            }
        }

        /* 移除坏三角形 */
        for (const auto& bad : badTriangles) {
            for (int t = 0; t < m_triangles.size(); ++t) {
                if (m_triangles[t].v0 == bad.v0 &&
                    m_triangles[t].v1 == bad.v1 &&
                    m_triangles[t].v2 == bad.v2) {
                    m_triangles.removeAt(t);
                    break;
                }
            }
        }

        /* 创建新三角形 */
        for (const auto& edge : polygon)
            m_triangles.append({edge.v0, edge.v1, i});
    }

    /* 去除超级三角形相关 */
    for (int t = m_triangles.size() - 1; t >= 0; --t) {
        const auto& tri = m_triangles[t];
        if (tri.v0 >= n || tri.v1 >= n || tri.v2 >= n)
            m_triangles.removeAt(t);
    }

    result = m_triangles;

    m_stats.totalTriangulated++;
    m_stats.totalTriangles += result.size();
    m_stats.totalPoints += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTriangulated;

    emit triangulationCompleted(result.size());
    return result;
}

void DelaunayTriangulation2::initIncremental(double superWidth, double superHeight)
{
    m_incrementalMode = true;
    m_points.clear();
    m_triangles.clear();

    double hw = superWidth / 2.0, hh = superHeight / 2.0;
    int p0 = 0, p1 = 1, p2 = 2;
    m_points.append({-hw * 20, -hh * 20});
    m_points.append({0, hh * 20});
    m_points.append({hw * 20, -hh * 20});
    m_triangles.append({p0, p1, p2});
}

void DelaunayTriangulation2::insertPoint(const Point2D& p)
{
    int idx = m_points.size();
    m_points.append(p);

    QVector<Triangle> badTriangles;
    for (const auto& tri : m_triangles) {
        if (inCircumCircle(p, m_points[tri.v0], m_points[tri.v1], m_points[tri.v2]))
            badTriangles.append(tri);
    }

    QVector<Edge> polygon;
    for (const auto& tri : badTriangles) {
        Edge edges[3] = {{tri.v0, tri.v1}, {tri.v1, tri.v2}, {tri.v2, tri.v0}};
        for (const auto& edge : edges) {
            bool shared = false;
            for (const auto& other : badTriangles) {
                if (&tri == &other) continue;
                Edge oE[3] = {{other.v0, other.v1}, {other.v1, other.v2}, {other.v2, other.v0}};
                for (const auto& oe : oE) {
                    if ((edge.v0 == oe.v0 && edge.v1 == oe.v1) ||
                        (edge.v0 == oe.v1 && edge.v1 == oe.v0)) {
                        shared = true; break;
                    }
                }
                if (shared) break;
            }
            if (!shared) polygon.append(edge);
        }
    }

    for (const auto& bad : badTriangles) {
        for (int t = 0; t < m_triangles.size(); ++t) {
            if (m_triangles[t].v0 == bad.v0 &&
                m_triangles[t].v1 == bad.v1 &&
                m_triangles[t].v2 == bad.v2) {
                m_triangles.removeAt(t);
                break;
            }
        }
    }

    for (const auto& edge : polygon)
        m_triangles.append({edge.v0, edge.v1, idx});
}

QVector<DelaunayTriangulation2::Triangle> DelaunayTriangulation2::getTriangles() const
{
    QVector<Triangle> result;
    int n = m_incrementalMode ? 3 : 0;
    for (const auto& tri : m_triangles) {
        if (tri.v0 >= n || tri.v1 >= n || tri.v2 >= n) continue;
        result.append(tri);
    }
    return result;
}

QVector<DelaunayTriangulation2::Edge> DelaunayTriangulation2::voronoiEdges(
    const QVector<Point2D>& points,
    const QVector<Triangle>& triangles) const
{
    Q_UNUSED(points)
    QVector<Edge> edges;

    for (int i = 0; i < triangles.size(); ++i) {
        for (int j = i + 1; j < triangles.size(); ++j) {
            const auto& a = triangles[i];
            const auto& b = triangles[j];
            int shared = 0;
            int vertsA[3] = {a.v0, a.v1, a.v2};
            int vertsB[3] = {b.v0, b.v1, b.v2};
            for (int va : vertsA)
                for (int vb : vertsB)
                    if (va == vb) shared++;
            if (shared == 2)
                edges.append({i, j});
        }
    }
    return edges;
}

bool DelaunayTriangulation2::inTriangle(const Point2D& p, const Point2D& a,
                                           const Point2D& b, const Point2D& c)
{
    double d1 = (p.x - b.x) * (a.y - b.y) - (a.x - b.x) * (p.y - b.y);
    double d2 = (p.x - c.x) * (b.y - c.y) - (b.x - c.x) * (p.y - c.y);
    double d3 = (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y);
    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

double DelaunayTriangulation2::circumCircleRadius(const Point2D& a, const Point2D& b,
                                                     const Point2D& c) const
{
    double ax = a.x, ay = a.y;
    double bx = b.x, by = b.y;
    double cx = c.x, cy = c.y;
    double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    if (std::abs(d) < 1e-15) return 1e15;
    double ux = ((ax*ax+ay*ay)*(by-cy)+(bx*bx+by*by)*(cy-ay)+(cx*cx+cy*cy)*(ay-by))/d;
    double uy = ((ax*ax+ay*ay)*(cx-bx)+(bx*bx+by*by)*(ax-cx)+(cx*cx+cy*cy)*(bx-ax))/d;
    return std::sqrt((ax-ux)*(ax-ux)+(ay-uy)*(ay-uy));
}

bool DelaunayTriangulation2::inCircumCircle(const Point2D& p, const Point2D& a,
                                               const Point2D& b, const Point2D& c) const
{
    double ax = a.x - p.x, ay = a.y - p.y;
    double bx = b.x - p.x, by = b.y - p.y;
    double cx = c.x - p.x, cy = c.y - p.y;
    double det = (ax*ax+ay*ay)*(bx*cy-cx*by)
               - (bx*bx+by*by)*(ax*cy-cx*ay)
               + (cx*cx+cy*cy)*(ax*by-bx*ay);
    /* 确保三角形朝外 */
    double orient = (a.x-b.x)*(c.y-b.y) - (a.y-b.y)*(c.x-b.x);
    return (orient > 0) ? (det > 0) : (det < 0);
}

DelaunayTriangulation2::Stats DelaunayTriangulation2::stats() const { return m_stats; }

void DelaunayTriangulation2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

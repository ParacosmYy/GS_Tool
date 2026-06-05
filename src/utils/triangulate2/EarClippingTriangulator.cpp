/**
 * @file EarClippingTriangulator.cpp
 * @brief Ear-Clipping多边形三角剖分实现
 */

#include "EarClippingTriangulator.h"
#include <QElapsedTimer>
#include <cmath>

EarClippingTriangulator::EarClippingTriangulator(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<EarClippingTriangulator::Triangle> EarClippingTriangulator::triangulate(
    const QVector<Point>& vertices)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Triangle> result;
    int n = vertices.size();
    if (n < 3) return result;

    /* 判断多边形方向 */
    double area = polygonArea(vertices);
    bool ccw = (area > 0);

    /* 构建索引链表 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    int remaining = n;
    int start = 0;

    while (remaining > 3) {
        bool earFound = false;

        for (int attempt = 0; attempt < remaining; ++attempt) {
            int curr = (start + attempt) % remaining;
            int prev = (curr - 1 + remaining) % remaining;
            int next = (curr + 1) % remaining;

            if (isConvexVertex(vertices[indices[prev]],
                               vertices[indices[curr]],
                               vertices[indices[next]], ccw)) {
                if (isEar(vertices, indices, curr, ccw)) {
                    Triangle tri;
                    tri.a = indices[prev];
                    tri.b = indices[curr];
                    tri.c = indices[next];
                    tri.area = triangleArea(vertices[tri.a], vertices[tri.b],
                                            vertices[tri.c]);
                    result.append(tri);

                    /* 移除当前顶点 */
                    indices.removeAt(curr);
                    remaining--;
                    start = prev % remaining;
                    earFound = true;
                    break;
                }
            }
        }

        if (!earFound) break;
    }

    /* 添加最后一个三角形 */
    if (remaining == 3) {
        Triangle tri;
        tri.a = indices[0];
        tri.b = indices[1];
        tri.c = indices[2];
        tri.area = triangleArea(vertices[tri.a], vertices[tri.b],
                                vertices[tri.c]);
        result.append(tri);
    }

    m_stats.totalTriangulations++;
    m_stats.totalTriangles += result.size();
    m_stats.totalVerticesProcessed += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTriangulations;

    emit triangulationCompleted(n, result.size());
    return result;
}

double EarClippingTriangulator::polygonArea(const QVector<Point>& vertices) const
{
    double area = 0.0;
    int n = vertices.size();
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += vertices[i].x * vertices[j].y;
        area -= vertices[j].x * vertices[i].y;
    }
    return area / 2.0;
}

bool EarClippingTriangulator::pointInTriangle(const Point& p, const Point& a,
                                               const Point& b,
                                               const Point& c) const
{
    double d1 = cross2d(p, a, b);
    double d2 = cross2d(p, b, c);
    double d3 = cross2d(p, c, a);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(hasNeg && hasPos);
}

double EarClippingTriangulator::triangleArea(const Point& a, const Point& b,
                                              const Point& c) const
{
    return std::abs(cross2d(a, b, c)) / 2.0;
}

bool EarClippingTriangulator::isConvex(const QVector<Point>& vertices) const
{
    int n = vertices.size();
    if (n < 3) return false;

    double area = polygonArea(vertices);
    bool ccw = (area > 0);

    for (int i = 0; i < n; ++i) {
        int prev = (i - 1 + n) % n;
        int next = (i + 1) % n;
        if (!isConvexVertex(vertices[prev], vertices[i], vertices[next], ccw))
            return false;
    }
    return true;
}

double EarClippingTriangulator::cross2d(const Point& o, const Point& a,
                                         const Point& b) const
{
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool EarClippingTriangulator::isConvexVertex(const Point& prev,
                                              const Point& curr,
                                              const Point& next,
                                              bool ccw) const
{
    double cross = cross2d(prev, curr, next);
    return ccw ? (cross > 0) : (cross < 0);
}

bool EarClippingTriangulator::isEar(const QVector<Point>& poly,
                                     const QVector<int>& indices,
                                     int i, bool ccw) const
{
    int n = indices.size();
    int prev = (i - 1 + n) % n;
    int next = (i + 1) % n;

    Point a = poly[indices[prev]];
    Point b = poly[indices[i]];
    Point c = poly[indices[next]];

    /* 检查是否有其他顶点在三角形内 */
    for (int j = 0; j < n; ++j) {
        if (j == prev || j == i || j == next) continue;
        if (pointInTriangle(poly[indices[j]], a, b, c))
            return false;
    }
    return true;
}

EarClippingTriangulator::Stats EarClippingTriangulator::stats() const
{
    return m_stats;
}

void EarClippingTriangulator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file ConvexHull3D.cpp
 * @brief 三维凸包实现
 */

#include "ConvexHull3D.h"
#include <QElapsedTimer>
#include <cmath>

double ConvexHull3D::tripleProduct(const Point3D& a, const Point3D& b,
                                     const Point3D& c, const Point3D& d) const
{
    double bx = b.x - a.x, by = b.y - a.y, bz = b.z - a.z;
    double cx = c.x - a.x, cy = c.y - a.y, cz = c.z - a.z;
    double dx = d.x - a.x, dy = d.y - a.y, dz = d.z - a.z;

    /* (b-a) × (c-a) · (d-a) */
    double crossX = by * cz - bz * cy;
    double crossY = bz * cx - bx * cz;
    double crossZ = bx * cy - by * cx;

    return crossX * dx + crossY * dy + crossZ * dz;
}

double ConvexHull3D::triangleArea(const Point3D& a, const Point3D& b,
                                    const Point3D& c) const
{
    double ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
    double vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;

    double cx = uy * vz - uz * vy;
    double cy = uz * vx - ux * vz;
    double cz = ux * vy - uy * vx;

    return 0.5 * std::sqrt(cx * cx + cy * cy + cz * cz);
}

QVector<ConvexHull3D::Face> ConvexHull3D::compute(const QVector<Point3D>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Face> faces;
    int n = points.size();

    if (n < 4) {
        m_stats.totalComputed++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;
        return faces;
    }

    /* 找一个初始四面体 */
    int p0 = 0, p1 = -1, p2 = -1, p3 = -1;

    /* 找不共线的p1 */
    for (int i = 1; i < n && p1 < 0; ++i) {
        double dx = points[i].x - points[p0].x;
        double dy = points[i].y - points[p0].y;
        double dz = points[i].z - points[p0].z;
        if (dx * dx + dy * dy + dz * dz > 1e-20) p1 = i;
    }
    if (p1 < 0) return faces;

    /* 找不共线的p2 */
    for (int i = 1; i < n && p2 < 0; ++i) {
        if (i == p0 || i == p1) continue;
        if (std::abs(tripleProduct(points[p0], points[p1], points[p0], points[i])) > 1e-20)
            p2 = i;
    }
    if (p2 < 0) return faces;

    /* 找不共面的p3 */
    for (int i = 1; i < n && p3 < 0; ++i) {
        if (i == p0 || i == p1 || i == p2) continue;
        if (std::abs(tripleProduct(points[p0], points[p1], points[p2], points[i])) > 1e-20)
            p3 = i;
    }
    if (p3 < 0) return faces;

    /* 确保朝外 */
    if (tripleProduct(points[p0], points[p1], points[p2], points[p3]) > 0)
        std::swap(p1, p2);

    /* 初始四面体的4个面 */
    faces.append({p0, p1, p2});
    faces.append({p0, p3, p1});
    faces.append({p0, p2, p3});
    faces.append({p1, p3, p2});

    /* 增量添加其余点 */
    for (int i = 0; i < n; ++i) {
        if (i == p0 || i == p1 || i == p2 || i == p3) continue;

        QVector<Face> newFaces;
        QVector<bool> visible(faces.size(), false);

        for (int f = 0; f < faces.size(); ++f) {
            double prod = tripleProduct(
                points[faces[f].v0], points[faces[f].v1],
                points[faces[f].v2], points[i]);
            visible[f] = (prod > 1e-15);
        }

        for (int f = 0; f < faces.size(); ++f) {
            if (!visible[f]) {
                newFaces.append(faces[f]);
                continue;
            }

            /* 对可见面的不可见邻居创建新面 */
            const auto& face = faces[f];
            int edges[3][2] = {{face.v0, face.v1}, {face.v1, face.v2}, {face.v2, face.v0}};

            for (int e = 0; e < 3; ++e) {
                int a = edges[e][0], b = edges[e][1];
                bool edgeVisible = false;

                for (int f2 = 0; f2 < faces.size(); ++f2) {
                    if (f2 == f || !visible[f2]) continue;
                    const auto& other = faces[f2];
                    if ((other.v0 == a || other.v1 == a || other.v2 == a) &&
                        (other.v0 == b || other.v1 == b || other.v2 == b)) {
                        edgeVisible = true;
                        break;
                    }
                }

                if (!edgeVisible) {
                    newFaces.append({a, b, i});
                }
            }
        }

        faces = newFaces;
    }

    m_stats.totalComputed++;
    m_stats.totalFaces += faces.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputed;

    emit computationCompleted(faces.size());
    return faces;
}

double ConvexHull3D::volume(const QVector<Point3D>& points,
                              const QVector<Face>& faces) const
{
    double vol = 0.0;
    Point3D origin{0, 0, 0};
    for (const auto& f : faces)
        vol += tripleProduct(origin, points[f.v0], points[f.v1], points[f.v2]);
    return std::abs(vol) / 6.0;
}

double ConvexHull3D::surfaceArea(const QVector<Point3D>& points,
                                   const QVector<Face>& faces) const
{
    double area = 0.0;
    for (const auto& f : faces)
        area += triangleArea(points[f.v0], points[f.v1], points[f.v2]);
    return area;
}

bool ConvexHull3D::isInside(const Point3D& point,
                               const QVector<Point3D>& hullPoints,
                               const QVector<Face>& faces) const
{
    for (const auto& f : faces) {
        if (tripleProduct(hullPoints[f.v0], hullPoints[f.v1],
                           hullPoints[f.v2], point) > 1e-10)
            return false;
    }
    return true;
}

ConvexHull3D::Stats ConvexHull3D::stats() const { return m_stats; }

void ConvexHull3D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

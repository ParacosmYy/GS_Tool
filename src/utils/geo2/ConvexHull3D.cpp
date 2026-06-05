/**
 * @file ConvexHull3D.cpp
 * @brief 三维凸包实现 — QuickHull算法
 */

#include "utils/geo2/ConvexHull3D.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QSet>
#include <algorithm>

ConvexHull3D::ConvexHull3D(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<ConvexHull3D::Face> ConvexHull3D::compute(
    const QVector<Point3D>& points)
{
    QElapsedTimer timer;
    timer.start();

    if (points.size() < 4) {
        return {};
    }

    /* 构建初始四面体 */
    QVector<Face> faces = buildInitialTetrahedron(points);
    if (faces.isEmpty()) {
        return {};
    }

    /* 计算每个面的法线 */
    for (auto& face : faces) {
        computeFaceNormal(face, points);
    }

    /* QuickHull迭代: 为每个面找最远的点, 分裂面 */
    QSet<int> processed;
    bool changed = true;
    int maxIterations = points.size() * 10;
    int iter = 0;

    while (changed && iter < maxIterations) {
        changed = false;
        iter++;

        QVector<Face> newFaces;

        for (int fi = 0; fi < faces.size(); ++fi) {
            Face& face = faces[fi];
            int farthest = -1;
            double maxDist = 0.0;

            /* 找到面外最远的点 */
            for (int pi = 0; pi < points.size(); ++pi) {
                Point3D v = points[pi] - points[face.a];
                double dist = v.dot(face.normal);

                /* 确保点在面外侧且不在面上 */
                if (dist > 1e-10 && dist > maxDist &&
                    !processed.contains(pi)) {
                    maxDist = dist;
                    farthest = pi;
                }
            }

            if (farthest < 0) continue;
            processed.insert(farthest);
            changed = true;

            /* 收集从最远点可见的面, 找到地平线边 */
            QVector<int> visibleFaces;
            visibleFaces.append(fi);

            for (int fj = 0; fj < faces.size(); ++fj) {
                if (fj == fi) continue;
                Point3D v = points[farthest] - points[faces[fj].a];
                if (v.dot(faces[fj].normal) > 1e-10) {
                    visibleFaces.append(fj);
                }
            }

            /* 从最远点与可见面的非共享边创建新面 */
            for (int vi : visibleFaces) {
                Face vf = faces[vi];
                int edges[3][2] = {{vf.a, vf.b}, {vf.b, vf.c}, {vf.c, vf.a}};

                for (int e = 0; e < 3; ++e) {
                    int p1 = edges[e][0], p2 = edges[e][1];

                    /* 检查此边是否与其他可见面共享 */
                    bool shared = false;
                    for (int vj : visibleFaces) {
                        if (vj == vi) continue;
                        Face& other = faces[vj];
                        int oe[3][2] = {{other.a, other.b},
                                        {other.b, other.c},
                                        {other.c, other.a}};
                        for (int oe2 = 0; oe2 < 3; ++oe2) {
                            if ((oe[oe2][0] == p1 && oe[oe2][1] == p2) ||
                                (oe[oe2][0] == p2 && oe[oe2][1] == p1)) {
                                shared = true;
                                break;
                            }
                        }
                        if (shared) break;
                    }

                    /* 非共享边 = 地平线边, 创建新面 */
                    if (!shared) {
                        Face nf(p1, p2, farthest);
                        computeFaceNormal(nf, points);
                        newFaces.append(nf);
                    }
                }
            }

            /* 标记可见面为无效(用空面替换) */
            for (int vi : visibleFaces) {
                faces[vi].a = faces[vi].b = faces[vi].c = -1;
            }
        }

        /* 移除无效面, 添加新面 */
        faces.erase(
            std::remove_if(faces.begin(), faces.end(),
                [](const Face& f) { return f.a < 0; }),
            faces.end());

        for (auto& nf : newFaces) {
            faces.append(nf);
        }
    }

    /* 重新计算法线和面积 */
    for (auto& face : faces) {
        computeFaceNormal(face, points);
    }

    /* 更新统计 */
    ++m_stats.totalHullsComputed;
    m_stats.totalInputPoints += points.size();
    m_stats.totalFaces += faces.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHullsComputed;

    int vertexCount = hullVertices(faces).size();
    emit hullComputed(vertexCount, faces.size());
    return faces;
}

bool ConvexHull3D::contains(const Point3D& point,
                             const QVector<Point3D>& points,
                             const QVector<Face>& faces) const
{
    ++m_stats.totalContainmentTests;

    /* 点在凸包内部 = 点在所有面的内侧 */
    for (const auto& face : faces) {
        Point3D v = point - points[face.a];
        double dist = v.dot(face.normal);
        if (dist > 1e-10) return false;
    }
    return true;
}

double ConvexHull3D::volume(const QVector<Point3D>& points,
                             const QVector<Face>& faces) const
{
    double vol = 0.0;
    Point3D origin(0, 0, 0);

    /* 使用有符号体积公式: V = (1/6) * sum(a · (b × c)) */
    for (const auto& face : faces) {
        Point3D a = points[face.a] - origin;
        Point3D b = points[face.b] - origin;
        Point3D c = points[face.c] - origin;
        vol += a.dot(b.cross(c));
    }
    return qAbs(vol) / 6.0;
}

double ConvexHull3D::surfaceArea(const QVector<Face>& faces) const
{
    double area = 0.0;
    for (const auto& face : faces) {
        area += face.area;
    }
    return area;
}

ConvexHull3D::Point3D ConvexHull3D::centroid(
    const QVector<Point3D>& points,
    const QVector<Face>& faces) const
{
    if (faces.isEmpty()) return {};

    Point3D center;
    QSet<int> verts;
    for (const auto& f : faces) {
        verts.insert(f.a);
        verts.insert(f.b);
        verts.insert(f.c);
    }
    for (int idx : verts) {
        center = center + points[idx];
    }
    return center * (1.0 / verts.size());
}

QVector<int> ConvexHull3D::hullVertices(const QVector<Face>& faces) const
{
    QSet<int> verts;
    for (const auto& f : faces) {
        verts.insert(f.a);
        verts.insert(f.b);
        verts.insert(f.c);
    }
    return QVector<int>(verts.begin(), verts.end());
}

ConvexHull3D::Stats ConvexHull3D::stats() const
{
    return m_stats;
}

void ConvexHull3D::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void ConvexHull3D::computeFaceNormal(Face& face,
                                      const QVector<Point3D>& points) const
{
    Point3D ab = points[face.b] - points[face.a];
    Point3D ac = points[face.c] - points[face.a];
    face.normal = ab.cross(ac);
    double len = face.normal.length();
    if (len > 1e-15) {
        face.normal = face.normal * (1.0 / len);
    }
    face.area = len * 0.5;
}

QVector<ConvexHull3D::Face> ConvexHull3D::buildInitialTetrahedron(
    const QVector<Point3D>& points)
{
    /* 寻找4个非共面点构建初始四面体 */
    int n = points.size();

    /* 找x/y/z方向最远的点对 */
    int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    for (int i = 1; i < n; ++i) {
        if (points[i].x < points[minX].x) minX = i;
        if (points[i].x > points[maxX].x) maxX = i;
        if (points[i].y < points[minY].y) minY = i;
        if (points[i].y > points[maxY].y) maxY = i;
        if (points[i].z < points[minZ].z) minZ = i;
        if (points[i].z > points[maxZ].z) maxZ = i;
    }

    /* 选择最长轴上的点对 */
    double dx = (points[maxX] - points[minX]).length();
    double dy = (points[maxY] - points[minY]).length();
    double dz = (points[maxZ] - points[minZ]).length();

    int p0, p1;
    if (dx >= dy && dx >= dz) { p0 = minX; p1 = maxX; }
    else if (dy >= dz)        { p0 = minY; p1 = maxY; }
    else                      { p0 = minZ; p1 = maxZ; }

    /* 找离p0-p1线段最远的点p2 */
    int p2 = -1;
    double maxDist = 0.0;
    Point3D lineDir = points[p1] - points[p0];
    double lineLen = lineDir.length();
    if (lineLen < 1e-15) return {};

    for (int i = 0; i < n; ++i) {
        if (i == p0 || i == p1) continue;
        Point3D v = points[i] - points[p0];
        double projLen = v.dot(lineDir) / lineLen;
        Point3D proj = points[p0] + lineDir * (projLen / lineLen);
        double dist = (points[i] - proj).length();
        if (dist > maxDist) { maxDist = dist; p2 = i; }
    }
    if (p2 < 0) return {};

    /* 找离p0-p1-p2平面最远的点p3 */
    int p3 = -1;
    maxDist = 0.0;
    Point3D normal = (points[p1] - points[p0]).cross(points[p2] - points[p0]);
    double normalLen = normal.length();
    if (normalLen < 1e-15) return {};

    for (int i = 0; i < n; ++i) {
        if (i == p0 || i == p1 || i == p2) continue;
        Point3D v = points[i] - points[p0];
        double dist = qAbs(v.dot(normal)) / normalLen;
        if (dist > maxDist) { maxDist = dist; p3 = i; }
    }
    if (p3 < 0) return {};

    /* 构建四面体的4个面, 确保法线朝外 */
    QVector<Face> faces;
    faces.append(Face(p0, p1, p2));
    faces.append(Face(p0, p2, p3));
    faces.append(Face(p0, p3, p1));
    faces.append(Face(p1, p3, p2));

    /* 调整法线方向, 使其朝外(远离质心) */
    Point3D center = (points[p0] + points[p1] +
                      points[p2] + points[p3]) * 0.25;

    for (auto& face : faces) {
        computeFaceNormal(face, points);
        Point3D faceCenter = (points[face.a] + points[face.b] +
                              points[face.c]) * (1.0 / 3.0);
        Point3D toCenter = center - faceCenter;
        if (toCenter.dot(face.normal) > 0) {
            /* 法线朝内, 翻转 */
            std::swap(face.b, face.c);
            face.normal = face.normal * (-1.0);
        }
    }

    return faces;
}

/**
 * @file VoronoiDiagram.cpp
 * @brief Voronoi图实现 — Delaunay对偶法
 */

#include "utils/voronoi2/VoronoiDiagram.h"

#include "utils/delaunay2/DelaunayFlip.h"

#include <QElapsedTimer>
#include <cmath>
#include <map>
#include <set>
#include <vector>

// ============================================================================
// 构造
// ============================================================================

VoronoiDual::VoronoiDual(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

QVector<QVector<QPair<double,double>>> VoronoiDual::compute(
    QVector<QPair<double,double>> sites)
{
    QElapsedTimer timer;
    timer.start();

    int n = sites.size();
    QVector<QVector<QPair<double,double>>> polygons(n);

    if (n < 2) {
        ++m_stats.totalComputations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computationCompleted(n);
        return polygons;
    }

    /* 1. 构建Delaunay三角剖分 */
    DelaunayFlip delaunay;
    auto triangles = delaunay.triangulate(sites);

    if (triangles.isEmpty()) {
        ++m_stats.totalComputations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
            ? m_timeSum / m_stats.totalComputations : 0.0;
        emit computationCompleted(n);
        return polygons;
    }

    /* 2. 计算每个三角形的外接圆心 */
    int triCount = triangles.size();
    QVector<QPair<double,double>> cc(triCount);

    for (int i = 0; i < triCount; ++i) {
        int a = triangles[i][0];
        int b = triangles[i][1];
        int c = triangles[i][2];
        cc[i] = circumcenter(sites[a], sites[b], sites[c]);
    }

    /* 3. 构建边-三角形映射: 每条边属于哪些三角形 */
    struct EdgeKey {
        int v1, v2;
        bool operator<(const EdgeKey& o) const {
            return (v1 < o.v1) || (v1 == o.v1 && v2 < o.v2);
        }
    };

    std::map<EdgeKey, std::vector<int>> edgeTriMap;
    for (int i = 0; i < triCount; ++i) {
        int verts[3] = {
            triangles[i][0],
            triangles[i][1],
            triangles[i][2]
        };
        for (int e = 0; e < 3; ++e) {
            int va = verts[e];
            int vb = verts[(e + 1) % 3];
            EdgeKey ek;
            ek.v1 = std::min(va, vb);
            ek.v2 = std::max(va, vb);
            edgeTriMap[ek].push_back(i);
        }
    }

    /* 4. 为每个站点构建Voronoi多边形 */
    /* 收集每个站点关联的三角形索引 */
    std::vector<std::vector<int>> siteTriangles(n);
    for (int i = 0; i < triCount; ++i) {
        int verts[3] = {
            triangles[i][0],
            triangles[i][1],
            triangles[i][2]
        };
        for (int v = 0; v < 3; ++v) {
            if (verts[v] >= 0 && verts[v] < n) {
                siteTriangles[verts[v]].push_back(i);
            }
        }
    }

    /* 对每个站点，按角度排序关联三角形的外接圆心 */
    for (int s = 0; s < n; ++s) {
        const auto& tris = siteTriangles[s];
        if (tris.empty()) {
            polygons[s] = {};
            continue;
        }

        double sx = sites[s].first;
        double sy = sites[s].second;

        /* 计算每个外接圆心相对于站点的角度 */
        std::vector<std::pair<double, int>> angleTri;
        angleTri.reserve(tris.size());
        for (int ti : tris) {
            double dx = cc[ti].first - sx;
            double dy = cc[ti].second - sy;
            double angle = std::atan2(dy, dx);
            angleTri.push_back({angle, ti});
        }

        /* 按角度排序(逆时针) */
        std::sort(angleTri.begin(), angleTri.end());

        /* 提取排序后的外接圆心 */
        polygons[s].reserve(static_cast<int>(angleTri.size()));
        for (const auto& at : angleTri) {
            polygons[s].append(cc[at.second]);
        }
    }

    ++m_stats.totalComputations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalComputations > 0)
        ? m_timeSum / m_stats.totalComputations : 0.0;

    emit computationCompleted(n);
    return polygons;
}

void VoronoiDual::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

QPair<double,double> VoronoiDual::circumcenter(
    const QPair<double,double>& p0,
    const QPair<double,double>& p1,
    const QPair<double,double>& p2)
{
    double ax = p0.first, ay = p0.second;
    double bx = p1.first, by = p1.second;
    double cx = p2.first, cy = p2.second;

    double D = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

    if (std::abs(D) < 1e-20) {
        /* 退化: 返回重心 */
        return {(ax + bx + cx) / 3.0, (ay + by + cy) / 3.0};
    }

    double a2 = ax * ax + ay * ay;
    double b2 = bx * bx + by * by;
    double c2 = cx * cx + cy * cy;

    double ux = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / D;
    double uy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / D;

    return {ux, uy};
}

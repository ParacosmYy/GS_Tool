/**
 * @file VoronoiDiagram.cpp
 * @brief Voronoi图实现 — 暴力法+最近邻查询
 */

#include "utils/voronoi/VoronoiDiagram.h"

#include <QElapsedTimer>
#include <cmath>
#include <limits>

VoronoiDiagram::VoronoiDiagram(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

void VoronoiDiagram::compute(const QVector<Point>& sites)
{
    QElapsedTimer timer;
    timer.start();

    m_sites = sites;
    m_edges.clear();

    if (sites.size() < 2) {
        m_stats.totalComputations++;
        emit diagramComputed(sites.size(), 0);
        return;
    }

    computeBruteForce();

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalComputations + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalComputations + m_stats.totalQueries) : 0.0;

    emit diagramComputed(sites.size(), m_edges.size());
}

void VoronoiDiagram::computeBruteForce()
{
    int n = m_sites.size();
    /* 对每对站点计算垂直平分线段 */
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            Point mid;
            mid.x = (m_sites[i].x + m_sites[j].x) / 2.0;
            mid.y = (m_sites[i].y + m_sites[j].y) / 2.0;

            double dx = m_sites[j].x - m_sites[i].x;
            double dy = m_sites[j].y - m_sites[i].y;
            double len = std::sqrt(dx * dx + dy * dy);
            if (len < 1e-12) continue;

            /* 法线方向(垂直平分线方向) */
            double nx = -dy / len;
            double ny = dx / len;

            /* 有限长度的边 */
            double ext = 500.0;
            Edge e;
            e.a.x = mid.x - nx * ext;
            e.a.y = mid.y - ny * ext;
            e.b.x = mid.x + nx * ext;
            e.b.y = mid.y + ny * ext;
            e.site1 = i;
            e.site2 = j;
            m_edges.append(e);
        }
    }
}

int VoronoiDiagram::nearestSite(double x, double y)
{
    QElapsedTimer timer;
    timer.start();

    if (m_sites.isEmpty()) return -1;

    int best = 0;
    double bestDist = std::numeric_limits<double>::max();
    for (int i = 0; i < m_sites.size(); ++i) {
        double dx = m_sites[i].x - x;
        double dy = m_sites[i].y - y;
        double dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalComputations + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalComputations + m_stats.totalQueries) : 0.0;

    return best;
}

void VoronoiDiagram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

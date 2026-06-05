/**
 * @file PointLocation.cpp
 * @brief 平面细分点定位实现 — slab方法
 */

#include "utils/point_location/PointLocation.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
PointLocation::PointLocation(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
PointLocation::~PointLocation() = default;

/**
 * @brief 从边集构建slab结构
 *
 * 收集所有边的端点x坐标，排序去重后形成slab边界。
 * 对每个slab确定穿过它的线段集合。
 */
void PointLocation::build(const QVector<Edge>& edges)
{
    QElapsedTimer timer;
    timer.start();

    m_edges = edges;
    m_slabs.clear();

    if (edges.isEmpty()) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalBuilds;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return;
    }

    /* 收集所有唯一x坐标作为slab边界 */
    QVector<double> xCoords;
    xCoords.reserve(edges.size() * 2);
    for (const Edge& e : edges) {
        xCoords.append(e.first.first);
        xCoords.append(e.second.first);
    }

    std::sort(xCoords.begin(), xCoords.end());
    xCoords.erase(std::unique(xCoords.begin(), xCoords.end()), xCoords.end());

    if (xCoords.size() < 2) {
        /* 所有边在同一个x坐标上，无法形成slab */
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalBuilds;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return;
    }

    /* 构建slab数组 */
    m_slabs.reserve(xCoords.size() - 1);
    for (int i = 0; i < xCoords.size() - 1; ++i) {
        Slab slab;
        slab.xLeft = xCoords[i];
        slab.xRight = xCoords[i + 1];

        /* 确定穿过该slab的线段 */
        double slabMid = (slab.xLeft + slab.xRight) * 0.5;
        for (int j = 0; j < edges.size(); ++j) {
            const Edge& e = edges[j];
            double eXMin = std::min(e.first.first, e.second.first);
            double eXMax = std::max(e.first.first, e.second.first);

            /* 线段跨越或包含该slab */
            if (eXMin <= slabMid && eXMax >= slabMid) {
                slab.edgeIndices.append(j);
            }
        }

        /* 按线段在slab中点处的y坐标排序 */
        std::sort(slab.edgeIndices.begin(), slab.edgeIndices.end(),
                  [this, slabMid](int a, int b) {
                      return edgeYAtX(a, slabMid) < edgeYAtX(b, slabMid);
                  });

        m_slabs.append(slab);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalBuilds;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 定位点所在区域
 *
 * 先用二分查找确定点所在的slab，
 * 然后在该slab的排序线段列表中二分查找确定区域编号。
 */
int PointLocation::locate(double x, double y)
{
    QElapsedTimer timer;
    timer.start();

    int region = -1;

    /* 二分查找slab */
    int slabIdx = -1;
    int lo = 0, hi = m_slabs.size() - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (x >= m_slabs[mid].xLeft && x < m_slabs[mid].xRight) {
            slabIdx = mid;
            break;
        } else if (x < m_slabs[mid].xLeft) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }

    if (slabIdx >= 0) {
        const Slab& slab = m_slabs[slabIdx];

        if (slab.edgeIndices.isEmpty()) {
            /* slab内无线段，只有一个区域 */
            region = 0;
        } else {
            /* 在排序线段列表中二分查找 */
            int below = 0;
            for (int i = 0; i < slab.edgeIndices.size(); ++i) {
                double edgeY = edgeYAtX(slab.edgeIndices[i], x);
                if (y >= edgeY) {
                    below = i + 1;
                } else {
                    break;
                }
            }
            region = below;
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit queryCompleted(region);
    return region;
}

/** @brief 重置统计信息 */
void PointLocation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算线段在指定x坐标处的y值
 *
 * 对于垂直线段(两端x相同)，返回端点的平均y值。
 * 否则通过线性插值计算。
 */
double PointLocation::edgeYAtX(int edgeIdx, double x) const
{
    const Edge& e = m_edges[edgeIdx];
    double x1 = e.first.first, y1 = e.first.second;
    double x2 = e.second.first, y2 = e.second.second;

    if (std::abs(x2 - x1) < 1e-12) {
        return (y1 + y2) * 0.5; /* 垂直线段 */
    }

    double t = (x - x1) / (x2 - x1);
    return y1 + t * (y2 - y1);
}

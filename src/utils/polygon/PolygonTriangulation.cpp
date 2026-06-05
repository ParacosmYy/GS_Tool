/**
 * @file PolygonTriangulation.cpp
 * @brief 多边形三角剖分实现 — 耳切法
 */

#include "utils/polygon/PolygonTriangulation.h"

#include <QElapsedTimer>
#include <cmath>

// ============================================================================
// 构造
// ============================================================================

PolygonTriangulation::PolygonTriangulation(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

QVector<QVector<int>> PolygonTriangulation::triangulate(
    QVector<QPair<double,double>> polygon)
{
    QElapsedTimer timer;
    timer.start();

    int n = polygon.size();
    QVector<QVector<int>> result;

    if (n < 3) {
        ++m_stats.totalTriangulations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTriangulations > 0)
            ? m_timeSum / m_stats.totalTriangulations : 0.0;
        emit triangulationCompleted(0);
        return result;
    }

    if (n == 3) {
        result.append({0, 1, 2});
        ++m_stats.totalTriangulations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTriangulations > 0)
            ? m_timeSum / m_stats.totalTriangulations : 0.0;
        emit triangulationCompleted(1);
        return result;
    }

    /* 确保顶点按逆时针排列 */
    double area = 0.0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += polygon[i].first * polygon[j].second;
        area -= polygon[j].first * polygon[i].second;
    }

    /* indices存储当前剩余顶点的索引 */
    QVector<int> indices;
    indices.reserve(n);
    if (area > 0) {
        /* 逆时针 */
        for (int i = 0; i < n; ++i) indices.append(i);
    } else {
        /* 顺时针: 反转 */
        for (int i = n - 1; i >= 0; --i) indices.append(i);
    }

    /* 耳切主循环 */
    int remaining = n;
    int safety = n * 3; // 安全计数器防止无限循环

    while (remaining > 3 && safety > 0) {
        --safety;
        bool earFound = false;

        for (int i = 0; i < remaining; ++i) {
            if (isEar(polygon, indices, i)) {
                /* 切掉耳朵: 三角形(prev, i, next) */
                int prev = (i - 1 + remaining) % remaining;
                int next = (i + 1) % remaining;

                result.append({indices[prev], indices[i], indices[next]});

                /* 从列表中移除第i个顶点 */
                indices.removeAt(i);
                --remaining;
                earFound = true;
                break;
            }
        }

        if (!earFound) {
            /* 未找到耳朵(退化情况)，强制切掉第一个顶点 */
            int prev = 0;
            int curr = 1 % remaining;
            int next = 2 % remaining;
            result.append({indices[prev], indices[curr], indices[next]});
            indices.removeAt(curr);
            --remaining;
        }
    }

    /* 剩下3个顶点构成最后一个三角形 */
    if (remaining == 3) {
        result.append({indices[0], indices[1], indices[2]});
    }

    ++m_stats.totalTriangulations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTriangulations > 0)
        ? m_timeSum / m_stats.totalTriangulations : 0.0;

    emit triangulationCompleted(result.size());
    return result;
}

void PolygonTriangulation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

double PolygonTriangulation::cross(const QPair<double,double>& o,
                                    const QPair<double,double>& a,
                                    const QPair<double,double>& b)
{
    return (a.first - o.first) * (b.second - o.second)
         - (b.first - o.first) * (a.second - o.second);
}

bool PolygonTriangulation::isEar(
    const QVector<QPair<double,double>>& polygon,
    const QVector<int>& indices, int i)
{
    int n = indices.size();
    if (n < 3) return false;

    int prev = (i - 1 + n) % n;
    int next = (i + 1) % n;

    const auto& p0 = polygon[indices[prev]];
    const auto& p1 = polygon[indices[i]];
    const auto& p2 = polygon[indices[next]];

    /* 检查是否为凸顶点(叉积>0表示逆时针转角) */
    double cr = cross(p0, p1, p2);
    if (cr < 1e-12) return false; // 凹顶点，不是耳朵

    /* 检查是否有其他顶点落在三角形内 */
    for (int j = 0; j < n; ++j) {
        if (j == prev || j == i || j == next) continue;
        if (pointInTriangle(polygon[indices[j]], p0, p1, p2)) {
            return false;
        }
    }

    return true;
}

bool PolygonTriangulation::pointInTriangle(
    const QPair<double,double>& p,
    const QPair<double,double>& p0,
    const QPair<double,double>& p1,
    const QPair<double,double>& p2)
{
    double d1 = cross(p0, p1, p);
    double d2 = cross(p1, p2, p);
    double d3 = cross(p2, p0, p);

    bool hasNeg = (d1 < -1e-12) || (d2 < -1e-12) || (d3 < -1e-12);
    bool hasPos = (d1 > 1e-12) || (d2 > 1e-12) || (d3 > 1e-12);

    return !(hasNeg && hasPos);
}

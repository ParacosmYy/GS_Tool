/**
 * @file DelaunayFlip.cpp
 * @brief Delaunay三角剖分实现 — 增量插入+Lawson边翻转
 */

#include "utils/delaunay2/DelaunayFlip.h"

#include <QElapsedTimer>
#include <cmath>
#include <limits>

// ============================================================================
// 构造
// ============================================================================

DelaunayFlip::DelaunayFlip(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

// ============================================================================
// 公开方法
// ============================================================================

QVector<QVector<int>> DelaunayFlip::triangulate(
    QVector<QPair<double,double>> points)
{
    QElapsedTimer timer;
    timer.start();

    int n = points.size();
    QVector<QVector<int>> result;

    if (n < 3) {
        ++m_stats.totalTriangulations;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTriangulations > 0)
            ? m_timeSum / m_stats.totalTriangulations : 0.0;
        emit triangulationCompleted(0, n);
        return result;
    }

    /* 计算包围盒 */
    double minX = points[0].first,  minY = points[0].second;
    double maxX = points[0].first,  maxY = points[0].second;
    for (int i = 1; i < n; ++i) {
        minX = std::min(minX, points[i].first);
        minY = std::min(minY, points[i].second);
        maxX = std::max(maxX, points[i].first);
        maxY = std::max(maxY, points[i].second);
    }

    double dx = maxX - minX;
    double dy = maxY - minY;
    double dmax = std::max(dx, dy, 1e-10);
    double midX = (minX + maxX) / 2.0;
    double midY = (minY + maxY) / 2.0;

    /* 添加超级三角形的三个虚拟点 */
    m_points = points;
    m_points.append({midX - 20.0 * dmax, midY - dmax});
    m_points.append({midX, midY + 20.0 * dmax});
    m_points.append({midX + 20.0 * dmax, midY - dmax});

    int sp1 = n, sp2 = n + 1, sp3 = n + 2;

    /* 初始化: 只有一个超级三角形 */
    m_tris.clear();
    m_ccX.clear(); m_ccY.clear(); m_ccR2.clear();

    Tri t0;
    t0.a = sp1; t0.b = sp2; t0.c = sp3;
    t0.adjA = -1; t0.adjB = -1; t0.adjC = -1;
    m_tris.append(t0);
    m_ccX.resize(1); m_ccY.resize(1); m_ccR2.resize(1);
    computeCircumcircle(0);

    /* 逐点插入 */
    for (int i = 0; i < n; ++i) {
        insertPoint(i);
    }

    /* 移除与超级三角形关联的三角形 */
    removeSuperTriangle();

    /* 收集结果 */
    for (int i = 0; i < m_tris.size(); ++i) {
        const Tri& tri = m_tris[i];
        if (tri.a >= 0 && tri.b >= 0 && tri.c >= 0) {
            result.append({tri.a, tri.b, tri.c});
        }
    }

    ++m_stats.totalTriangulations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTriangulations > 0)
        ? m_timeSum / m_stats.totalTriangulations : 0.0;

    emit triangulationCompleted(result.size(), n);
    return result;
}

void DelaunayFlip::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ============================================================================
// 内部实现
// ============================================================================

double DelaunayFlip::crossProduct(const QPair<double,double>& p0,
                                   const QPair<double,double>& p1,
                                   const QPair<double,double>& p2)
{
    return (p1.first - p0.first) * (p2.second - p0.second)
         - (p2.first - p0.first) * (p1.second - p0.second);
}

bool DelaunayFlip::inCircumcircle(int pi, int ti)
{
    double dx = m_points[pi].first - m_ccX[ti];
    double dy = m_points[pi].second - m_ccY[ti];
    double dist2 = dx * dx + dy * dy;
    return dist2 < m_ccR2[ti];
}

void DelaunayFlip::computeCircumcircle(int ti)
{
    const auto& pa = m_points[m_tris[ti].a];
    const auto& pb = m_points[m_tris[ti].b];
    const auto& pc = m_points[m_tris[ti].c];

    double D = 2.0 * (pa.first * (pb.second - pc.second)
                    + pb.first * (pc.second - pa.second)
                    + pc.first * (pa.second - pb.second));

    if (std::abs(D) < 1e-20) {
        /* 退化三角形: 设置很大的外接圆 */
        m_ccX[ti] = (pa.first + pb.first + pc.first) / 3.0;
        m_ccY[ti] = (pa.second + pb.second + pc.second) / 3.0;
        m_ccR2[ti] = 1e30;
        return;
    }

    double a2 = pa.first * pa.first + pa.second * pa.second;
    double b2 = pb.first * pb.first + pb.second * pb.second;
    double c2 = pc.first * pc.first + pc.second * pc.second;

    double ux = (a2 * (pb.second - pc.second)
               + b2 * (pc.second - pa.second)
               + c2 * (pa.second - pb.second)) / D;
    double uy = (a2 * (pc.first - pb.first)
               + b2 * (pa.first - pc.first)
               + c2 * (pb.first - pa.first)) / D;

    m_ccX[ti] = ux;
    m_ccY[ti] = uy;
    double rdx = pa.first - ux;
    double rdy = pa.second - uy;
    m_ccR2[ti] = rdx * rdx + rdy * rdy;
}

void DelaunayFlip::insertPoint(int pi)
{
    int nTri = m_tris.size();
    QVector<int> badTri;

    /* 找到所有外接圆包含新点的三角形(坏三角形) */
    for (int i = 0; i < nTri; ++i) {
        if (inCircumcircle(pi, i)) {
            badTri.append(i);
        }
    }

    /* 找到多边形边界(坏三角形集合的边界边) */
    QVector<Edge> boundary;
    for (int bi = 0; bi < badTri.size(); ++bi) {
        int ti = badTri[bi];
        int edges[3][2] = {
            {m_tris[ti].a, m_tris[ti].b},
            {m_tris[ti].b, m_tris[ti].c},
            {m_tris[ti].c, m_tris[ti].a}
        };

        for (int e = 0; e < 3; ++e) {
            int v1 = edges[e][0];
            int v2 = edges[e][1];

            /* 检查这条边是否被其他坏三角形共享 */
            bool shared = false;
            for (int bj = 0; bj < badTri.size(); ++bj) {
                if (bi == bj) continue;
                int tj = badTri[bj];
                int e2[3][2] = {
                    {m_tris[tj].a, m_tris[tj].b},
                    {m_tris[tj].b, m_tris[tj].c},
                    {m_tris[tj].c, m_tris[tj].a}
                };
                for (int k = 0; k < 3; ++k) {
                    if ((e2[k][0] == v1 && e2[k][1] == v2) ||
                        (e2[k][0] == v2 && e2[k][1] == v1)) {
                        shared = true;
                        break;
                    }
                }
                if (shared) break;
            }

            if (!shared) {
                boundary.append({v1, v2});
            }
        }
    }

    /* 标记坏三角形为无效(置空) */
    for (int ti : badTri) {
        m_tris[ti].a = -1; m_tris[ti].b = -1; m_tris[ti].c = -1;
        m_tris[ti].adjA = -1; m_tris[ti].adjB = -1; m_tris[ti].adjC = -1;
    }

    /* 为每条边界边创建新三角形 */
    int firstNew = m_tris.size();
    for (int bi = 0; bi < boundary.size(); ++bi) {
        Tri nt;
        nt.a = boundary[bi].v1;
        nt.b = boundary[bi].v2;
        nt.c = pi;
        nt.adjA = -1; nt.adjB = -1; nt.adjC = -1;
        m_tris.append(nt);
        m_ccX.append(0.0); m_ccY.append(0.0); m_ccR2.append(0.0);
        computeCircumcircle(m_tris.size() - 1);
    }

    /* 设置新三角形之间的邻接关系 */
    int newCount = boundary.size();
    for (int i = 0; i < newCount; ++i) {
        int ni = firstNew + i;
        int v1 = boundary[i].v1;
        int v2 = boundary[i].v2;

        /* 邻接边: 对面的边 */
        /* adjA: a-b边的邻居 */
        /* adjB: b-c边的邻居 */
        /* adjC: c-a边的邻居 */
        for (int j = 0; j < newCount; ++j) {
            if (i == j) continue;
            int nj = firstNew + j;
            /* 检查边共享 */
            int vals[3] = {m_tris[nj].a, m_tris[nj].b, m_tris[nj].c};
            bool hasV1 = (vals[0] == v1 || vals[1] == v1 || vals[2] == v1);
            bool hasV2 = (vals[0] == v2 || vals[1] == v2 || vals[2] == v2);
            if (hasV1 && hasV2) {
                m_tris[ni].adjA = nj;
                break;
            }
        }

        for (int j = 0; j < newCount; ++j) {
            if (i == j) continue;
            int nj = firstNew + j;
            int bPt = m_tris[ni].b;
            int cPt = m_tris[ni].c;
            int vals[3] = {m_tris[nj].a, m_tris[nj].b, m_tris[nj].c};
            bool hasB = (vals[0] == bPt || vals[1] == bPt || vals[2] == bPt);
            bool hasC = (vals[0] == cPt || vals[1] == cPt || vals[2] == cPt);
            if (hasB && hasC) {
                m_tris[ni].adjB = nj;
                break;
            }
        }

        for (int j = 0; j < newCount; ++j) {
            if (i == j) continue;
            int nj = firstNew + j;
            int cPt = m_tris[ni].c;
            int aPt = m_tris[ni].a;
            int vals[3] = {m_tris[nj].a, m_tris[nj].b, m_tris[nj].c};
            bool hasC = (vals[0] == cPt || vals[1] == cPt || vals[2] == cPt);
            bool hasA = (vals[0] == aPt || vals[1] == aPt || vals[2] == aPt);
            if (hasC && hasA) {
                m_tris[ni].adjC = nj;
                break;
            }
        }
    }
}

void DelaunayFlip::removeSuperTriangle()
{
    int n = m_points.size() - 3; // 原始点数

    for (int i = m_tris.size() - 1; i >= 0; --i) {
        const Tri& t = m_tris[i];
        if (t.a >= n || t.b >= n || t.c >= n ||
            t.a < 0 || t.b < 0 || t.c < 0) {
            m_tris.removeAt(i);
        }
    }
}

void DelaunayFlip::flipEdge(int ti, int ei)
{
    /* 占位: Lawson翻转在insertPoint中通过坏三角形重三角化隐式完成 */
    Q_UNUSED(ti)
    Q_UNUSED(ei)
}

/**
 * @file FenwickTree5.cpp
 * @brief FenwickTree5 实现
 *
 * 实现三维树状数组：批量点更新、矩形/长方体范围查询。
 */

#include "utils/tree193/FenwickTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FenwickTree5::FenwickTree5(int nx, int ny, int nz, QObject *parent)
    : QObject(parent), m_nx(nx), m_ny(ny), m_nz(nz)
{
    resize(nx, ny, nz);
}

FenwickTree5::~FenwickTree5() = default;

/* ---- Resize ---- */

void FenwickTree5::resize(int nx, int ny, int nz)
{
    m_nx = qMax(0, nx);
    m_ny = qMax(0, ny);
    m_nz = qMax(0, nz);

    // BIT uses 1-indexed, so allocate n+1
    m_tree.resize(m_nx + 1);
    for (int x = 0; x <= m_nx; ++x) {
        m_tree[x].resize(m_ny + 1);
        for (int y = 0; y <= m_ny; ++y)
            m_tree[x][y].resize(m_nz + 1, 0.0);
    }

    m_stats.dimX = m_nx;
    m_stats.dimY = m_ny;
    m_stats.dimZ = m_nz;
}

/* ---- Validate coordinates ---- */

bool FenwickTree5::validCoord(int x, int y, int z) const
{
    return x >= 1 && x <= m_nx && y >= 1 && y <= m_ny && z >= 1 && z <= m_nz;
}

/* ---- Internal 3D update ---- */

void FenwickTree5::updateInternal(int x, int y, int z, double delta)
{
    for (int i = x; i <= m_nx; i += i & (-i))
        for (int j = y; j <= m_ny; j += j & (-j))
            for (int k = z; k <= m_nz; k += k & (-k))
                m_tree[i][j][k] += delta;
}

/* ---- Internal 3D prefix sum ---- */

double FenwickTree5::queryInternal(int x, int y, int z) const
{
    double sum = 0.0;
    for (int i = x; i > 0; i -= i & (-i))
        for (int j = y; j > 0; j -= j & (-j))
            for (int k = z; k > 0; k -= k & (-k))
                sum += m_tree[i][j][k];
    return sum;
}

/* ---- Point update ---- */

void FenwickTree5::pointUpdate(int x, int y, int z, double delta)
{
    QElapsedTimer timer;
    timer.start();

    if (!validCoord(x, y, z)) return;
    updateInternal(x, y, z, delta);

    m_stats.totalOperations++;
    m_stats.pointUpdates++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("update", 1, timer.elapsed());
}

/* ---- Batch update ---- */

void FenwickTree5::batchUpdate(
    const QVector<QPair<QVector<int>, double>>& updates)
{
    QElapsedTimer timer;
    timer.start();

    int count = 0;
    for (const auto& upd : updates) {
        if (upd.first.size() != 3) continue;
        int x = upd.first[0], y = upd.first[1], z = upd.first[2];
        if (!validCoord(x, y, z)) continue;
        updateInternal(x, y, z, upd.second);
        count++;
    }

    m_stats.totalOperations += count;
    m_stats.pointUpdates += count;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted("batch", count, timer.elapsed());
}

/* ---- Prefix sum from (1,1,1) to (x,y,z) ---- */

double FenwickTree5::prefixSum(int x, int y, int z) const
{
    x = qBound(0, x, m_nx);
    y = qBound(0, y, m_ny);
    z = qBound(0, z, m_nz);
    return queryInternal(x, y, z);
}

/* ---- Range query over box ---- */

double FenwickTree5::rangeQuery(int x1, int y1, int z1,
                                  int x2, int y2, int z2) const
{
    QElapsedTimer timer;
    timer.start();

    // Clamp to valid range
    x1 = qMax(1, x1); y1 = qMax(1, y1); z1 = qMax(1, z1);
    x2 = qMin(m_nx, x2); y2 = qMin(m_ny, y2); z2 = qMin(m_nz, z2);

    if (x1 > x2 || y1 > y2 || z1 > z2) return 0.0;

    // Inclusion-exclusion for 3D range
    double result = queryInternal(x2, y2, z2)
                  - queryInternal(x1 - 1, y2, z2)
                  - queryInternal(x2, y1 - 1, z2)
                  - queryInternal(x2, y2, z1 - 1)
                  + queryInternal(x1 - 1, y1 - 1, z2)
                  + queryInternal(x1 - 1, y2, z1 - 1)
                  + queryInternal(x2, y1 - 1, z1 - 1)
                  - queryInternal(x1 - 1, y1 - 1, z1 - 1);

    // const_cast for stats only
    const_cast<FenwickTree5*>(this)->m_stats.totalOperations++;
    const_cast<FenwickTree5*>(this)->m_stats.rangeQueries++;
    double elapsed = timer.elapsed();
    const_cast<FenwickTree5*>(this)->m_timeSum += elapsed;
    const_cast<FenwickTree5*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOperations;

    const_cast<FenwickTree5*>(this)->operationCompleted("query", 1, elapsed);
    return result;
}

/* ---- Point query ---- */

double FenwickTree5::pointQuery(int x, int y, int z) const
{
    if (!validCoord(x, y, z)) return 0.0;
    // Point query = prefix(x,y,z) - prefix(x-1,y,z) - ... + ...
    // Simpler: use range query over single point
    return rangeQuery(x, y, z, x, y, z);
}

/* ---- Clear ---- */

void FenwickTree5::clear()
{
    for (int x = 0; x <= m_nx; ++x)
        for (int y = 0; y <= m_ny; ++y)
            for (int z = 0; z <= m_nz; ++z)
                m_tree[x][y][z] = 0.0;
}

/* ---- Reset ---- */

void FenwickTree5::resetStatistics()
{
    m_stats = Stats{};
    m_stats.dimX = m_nx;
    m_stats.dimY = m_ny;
    m_stats.dimZ = m_nz;
    m_timeSum = 0.0;
    clear();
}

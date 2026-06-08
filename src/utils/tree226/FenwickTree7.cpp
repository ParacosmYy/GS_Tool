/**
 * @file FenwickTree7.cpp
 * @brief FenwickTree7 实现
 *
 * 实现3D树状数组范围查询、Z序曲线(Morton码)索引空间体积查询。
 */

#include "utils/tree226/FenwickTree7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FenwickTree7::FenwickTree7(QObject *parent) : QObject(parent) {}
FenwickTree7::~FenwickTree7() = default;

/* ---- Initialize 3D Fenwick tree ---- */

bool FenwickTree7::init(int dimX, int dimY, int dimZ)
{
    if (dimX < 1 || dimY < 1 || dimZ < 1) return false;

    m_dimX = dimX;
    m_dimY = dimY;
    m_dimZ = dimZ;

    // 3D BIT (1-indexed): [dimX+1][dimY+1][dimZ+1]
    m_tree.resize(dimX + 1);
    for (int i = 0; i <= dimX; ++i) {
        m_tree[i].resize(dimY + 1);
        for (int j = 0; j <= dimY; ++j)
            m_tree[i][j].resize(dimZ + 1, 0.0);
    }

    m_stats.dimX = dimX;
    m_stats.dimY = dimY;
    m_stats.dimZ = dimZ;
    return true;
}

/* ---- Point update ---- */

void FenwickTree7::update(int x, int y, int z, double value)
{
    QElapsedTimer timer;
    timer.start();

    // 3D BIT update: cascade through all indices
    for (int i = x + 1; i <= m_dimX; i += lsb(i)) {
        for (int j = y + 1; j <= m_dimY; j += lsb(j)) {
            for (int k = z + 1; k <= m_dimZ; k += lsb(k)) {
                m_tree[i][j][k] += value;
            }
        }
    }

    m_stats.numUpdates++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit operationCompleted("update", timer.elapsed());
}

/* ---- 3D prefix sum ---- */

double FenwickTree7::prefixSum(int x, int y, int z) const
{
    double sum = 0.0;
    for (int i = x + 1; i > 0; i -= lsb(i)) {
        for (int j = y + 1; j > 0; j -= lsb(j)) {
            for (int k = z + 1; k > 0; k -= lsb(k)) {
                sum += m_tree[i][j][k];
            }
        }
    }
    return sum;
}

/* ---- 3D range sum query ---- */

double FenwickTree7::rangeSum(int x1, int y1, int z1, int x2, int y2, int z2) const
{
    QElapsedTimer timer;
    timer.start();

    // Inclusion-exclusion for 3D range
    double result = prefixSum(x2, y2, z2)
                  - prefixSum(x1 - 1, y2, z2)
                  - prefixSum(x2, y1 - 1, z2)
                  - prefixSum(x2, y2, z1 - 1)
                  + prefixSum(x1 - 1, y1 - 1, z2)
                  + prefixSum(x1 - 1, y2, z1 - 1)
                  + prefixSum(x2, y1 - 1, z1 - 1)
                  - prefixSum(x1 - 1, y1 - 1, z1 - 1);

    const_cast<FenwickTree7*>(this)->m_stats.numQueries++;
    const_cast<FenwickTree7*>(this)->m_stats.totalOps++;
    const_cast<FenwickTree7*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Spread bits for Morton encoding ---- */

quint64 FenwickTree7::spreadBits(quint32 v) const
{
    quint64 x = v;
    x = (x | (x << 16)) & 0x0000FFFF0000FFFFULL;
    x = (x | (x << 8))  & 0x00FF00FF00FF00FFULL;
    x = (x | (x << 4))  & 0x0F0F0F0F0F0F0F0FULL;
    x = (x | (x << 2))  & 0x3333333333333333ULL;
    x = (x | (x << 1))  & 0x5555555555555555ULL;
    return x;
}

/* ---- Compact bits for Morton decoding ---- */

quint32 FenwickTree7::compactBits(quint64 v) const
{
    quint64 x = v;
    x = (x & 0x5555555555555555ULL) | ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) | ((x >> 2) & 0x3333333333333333ULL);
    x = (x & 0x0F0F0F0F0F0F0F0FULL) | ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
    x = (x & 0x00FF00FF00FF00FFULL) | ((x >> 8) & 0x00FF00FF00FF00FFULL);
    x = (x & 0x0000FFFF0000FFFFULL) | ((x >> 16) & 0x0000FFFF0000FFFFULL);
    return static_cast<quint32>(x & 0xFFFFFFFF);
}

/* ---- Morton encode (3D) ---- */

quint64 FenwickTree7::mortonEncode(int x, int y, int z) const
{
    return spreadBits(static_cast<quint32>(x)) |
          (spreadBits(static_cast<quint32>(y)) << 1) |
          (spreadBits(static_cast<quint32>(z)) << 2);
}

/* ---- Morton decode (3D) ---- */

void FenwickTree7::mortonDecode(quint64 code, int& x, int& y, int& z) const
{
    x = compactBits(code);
    y = compactBits(code >> 1);
    z = compactBits(code >> 2);
}

/* ---- Build Z-order Fenwick ---- */

void FenwickTree7::buildZFenwick()
{
    if (m_zOrderData.isEmpty()) return;

    // Sort by Morton code
    std::sort(m_zOrderData.begin(), m_zOrderData.end(),
              [](const QPair<quint64, double>& a, const QPair<quint64, double>& b) {
                  return a.first < b.first;
              });

    int n = m_zOrderData.size();
    m_zFenwick.resize(n + 1, 0.0);
    for (int i = 0; i < n; ++i)
        zUpdate(i + 1, m_zOrderData[i].second);
}

/* ---- Z-order prefix sum ---- */

double FenwickTree7::zPrefixSum(int idx) const
{
    double sum = 0.0;
    while (idx > 0) {
        sum += m_zFenwick[idx];
        idx -= lsb(idx);
    }
    return sum;
}

/* ---- Z-order update ---- */

void FenwickTree7::zUpdate(int idx, double delta)
{
    while (idx < m_zFenwick.size()) {
        m_zFenwick[idx] += delta;
        idx += lsb(idx);
    }
}

/* ---- Z-order range query ---- */

double FenwickTree7::zOrderRangeSum(quint64 mortonStart, quint64 mortonEnd) const
{
    QElapsedTimer timer;
    timer.start();

    // Binary search for Morton range boundaries
    int lo = 0, hi = m_zOrderData.size() - 1;
    int startIdx = m_zOrderData.size(), endIdx = -1;

    for (int i = 0; i < m_zOrderData.size(); ++i) {
        if (m_zOrderData[i].first >= mortonStart) { startIdx = i; break; }
    }
    for (int i = m_zOrderData.size() - 1; i >= 0; --i) {
        if (m_zOrderData[i].first <= mortonEnd) { endIdx = i; break; }
    }

    double result = 0.0;
    if (startIdx <= endIdx) {
        result = zPrefixSum(endIdx + 1) - zPrefixSum(startIdx);
    }

    const_cast<FenwickTree7*>(this)->m_stats.numZQueries++;
    const_cast<FenwickTree7*>(this)->m_stats.totalOps++;
    const_cast<FenwickTree7*>(this)->m_timeSum += timer.elapsed();
    const_cast<FenwickTree7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Bulk load ---- */

void FenwickTree7::bulkLoad(const QVector<QVector<QVector<double>>>& data)
{
    int dx = data.size();
    int dy = (dx > 0) ? data[0].size() : 0;
    int dz = (dy > 0) ? data[0][0].size() : 0;
    if (dx < 1 || dy < 1 || dz < 1) return;

    init(dx, dy, dz);

    // Load into 3D BIT using point updates
    for (int x = 0; x < dx; ++x) {
        for (int y = 0; y < dy; ++y) {
            for (int z = 0; z < dz; ++z) {
                update(x, y, z, data[x][y][z]);
            }
        }
    }

    // Build Z-order index
    m_zOrderData.clear();
    for (int x = 0; x < dx; ++x) {
        for (int y = 0; y < dy; ++y) {
            for (int z = 0; z < dz; ++z) {
                quint64 morton = mortonEncode(x, y, z);
                m_zOrderData.append(qMakePair(morton, data[x][y][z]));
            }
        }
    }
    buildZFenwick();
}

/* ---- Reset ---- */

void FenwickTree7::resetStatistics()
{
    m_tree.clear();
    m_zOrderData.clear();
    m_zFenwick.clear();
    m_dimX = m_dimY = m_dimZ = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}

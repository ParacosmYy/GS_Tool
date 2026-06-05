/**
 * @file HilbertCurve.cpp
 * @brief Hilbert空间填充曲线实现
 */

#include "HilbertCurve.h"
#include <QElapsedTimer>
#include <algorithm>

HilbertCurve::HilbertCurve(int order, QObject* parent)
    : QObject(parent)
    , m_order(qBound(1, order, 16))
    , m_timeSum(0.0)
{
}

void HilbertCurve::rotate(int n, int* x, int* y, int rx, int ry) const
{
    if (ry == 0) {
        if (rx == 1) {
            *x = n - 1 - *x;
            *y = n - 1 - *y;
        }
        int tmp = *x;
        *x = *y;
        *y = tmp;
    }
}

int HilbertCurve::encode(int x, int y) const
{
    QElapsedTimer timer;
    timer.start();

    int n = 1 << m_order;
    int d = 0;
    int tx = x, ty = y;

    for (int s = n / 2; s > 0; s /= 2) {
        int rx = (tx & s) ? 1 : 0;
        int ry = (ty & s) ? 1 : 0;
        d += s * s * ((3 * rx) ^ ry);
        rotate(s, &tx, &ty, rx, ry);
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodeCompleted(x, y, d);
    return d;
}

QPair<int, int> HilbertCurve::decode(int index) const
{
    QElapsedTimer timer;
    timer.start();

    int n = 1 << m_order;
    int tx = 0, ty = 0;

    for (int s = 1; s < n; s *= 2) {
        int rx = 1 & (index / 2);
        int ry = 1 & (index ^ rx);
        rotate(s, &tx, &ty, rx, ry);
        tx += s * rx;
        ty += s * ry;
        index /= 4;
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return {tx, ty};
}

QVector<int> HilbertCurve::encodeBatch(const QVector<QPair<int, int>>& points) const
{
    QVector<int> result;
    result.reserve(points.size());
    for (const auto& p : points)
        result.append(encode(p.first, p.second));
    return result;
}

QVector<QPair<int, int>> HilbertCurve::decodeBatch(const QVector<int>& indices) const
{
    QVector<QPair<int, int>> result;
    result.reserve(indices.size());
    for (int idx : indices)
        result.append(decode(idx));
    return result;
}

int HilbertCurve::curveDistance(int idx1, int idx2) const
{
    return std::abs(idx1 - idx2);
}

QVector<QPair<int, int>> HilbertCurve::curveRange(int fromIdx, int toIdx) const
{
    QVector<QPair<int, int>> result;
    if (fromIdx > toIdx) std::swap(fromIdx, toIdx);
    for (int i = fromIdx; i <= toIdx; ++i)
        result.append(decode(i));
    return result;
}

int HilbertCurve::gridSize() const { return 1 << m_order; }
int HilbertCurve::order() const { return m_order; }
HilbertCurve::Stats HilbertCurve::stats() const { return m_stats; }

void HilbertCurve::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

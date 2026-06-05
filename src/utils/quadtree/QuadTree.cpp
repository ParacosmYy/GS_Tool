/**
 * @file QuadTree.cpp
 * @brief 四叉树 — 二维空间索引实现
 */

#include "QuadTree.h"
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

QuadTree::QuadTree(const Rect& bounds, int capacity, QObject* parent)
    : QObject(parent)
    , m_bounds(bounds)
    , m_capacity(capacity > 0 ? capacity : 4)
    , m_divided(false)
    , m_nw(nullptr)
    , m_ne(nullptr)
    , m_sw(nullptr)
    , m_se(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

QuadTree::~QuadTree()
{
    delete m_nw;
    delete m_ne;
    delete m_sw;
    delete m_se;
}

bool QuadTree::insert(const Point& point)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_bounds.contains(point)) return false;

    if (m_points.size() < m_capacity && !m_divided) {
        m_points.append(point);
        m_stats.totalInserts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum /
            (m_stats.totalInserts + m_stats.totalQueries);

        emit pointInserted(point.x, point.y);
        return true;
    }

    if (!m_divided) subdivide();

    if (m_nw->insert(point)) return true;
    if (m_ne->insert(point)) return true;
    if (m_sw->insert(point)) return true;
    if (m_se->insert(point)) return true;

    return false;
}

void QuadTree::subdivide()
{
    double hw = m_bounds.w / 2.0;
    double hh = m_bounds.h / 2.0;

    Rect nwR = {m_bounds.x, m_bounds.y, hw, hh};
    Rect neR = {m_bounds.x + hw, m_bounds.y, hw, hh};
    Rect swR = {m_bounds.x, m_bounds.y + hh, hw, hh};
    Rect seR = {m_bounds.x + hw, m_bounds.y + hh, hw, hh};

    m_nw = new QuadTree(nwR, m_capacity, this);
    m_ne = new QuadTree(neR, m_capacity, this);
    m_sw = new QuadTree(swR, m_capacity, this);
    m_se = new QuadTree(seR, m_capacity, this);
    m_divided = true;
    m_stats.totalNodes += 4;

    /* 重新分配现有点 */
    for (const auto& p : m_points) {
        m_nw->insert(p) || m_ne->insert(p) || m_sw->insert(p) || m_se->insert(p);
    }
    m_points.clear();
}

QVector<QuadTree::Point> QuadTree::queryRange(const Rect& range) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<Point> result;
    queryRangeImpl(range, result);

    const_cast<QuadTree*>(this)->m_stats.totalQueries++;
    const_cast<QuadTree*>(this)->m_timeSum += timer.elapsed();
    const_cast<QuadTree*>(this)->m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInserts + m_stats.totalQueries);

    const_cast<QuadTree*>(this)->rangeQueryCompleted(result.size());
    return result;
}

void QuadTree::queryRangeImpl(const Rect& range, QVector<Point>& result) const
{
    if (!m_bounds.intersects(range)) return;

    for (const auto& p : m_points) {
        if (range.contains(p)) result.append(p);
    }

    if (m_divided) {
        m_nw->queryRangeImpl(range, result);
        m_ne->queryRangeImpl(range, result);
        m_sw->queryRangeImpl(range, result);
        m_se->queryRangeImpl(range, result);
    }
}

QVector<QuadTree::Point> QuadTree::nearestNeighbors(double x, double y, int k) const
{
    QElapsedTimer timer;
    timer.start();

    /* 先收集所有点，排序取前k个(简化实现) */
    QVector<QPair<double, Point>> all;
    Rect full = {m_bounds.x - 1000, m_bounds.y - 1000,
                 m_bounds.w + 2000, m_bounds.h + 2000};
    QVector<Point> pts = queryRange(full);
    for (const auto& p : pts) {
        double dist = (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);
        all.append({dist, p});
    }

    std::sort(all.begin(), all.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<Point> result;
    for (int i = 0; i < qMin(k, all.size()); ++i) {
        result.append(all[i].second);
    }

    const_cast<QuadTree*>(this)->m_timeSum += timer.elapsed();
    return result;
}

int QuadTree::size() const
{
    int count = m_points.size();
    if (m_divided) {
        count += m_nw->size() + m_ne->size() + m_sw->size() + m_se->size();
    }
    return count;
}

void QuadTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

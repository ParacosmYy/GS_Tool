/**
 * @file IntervalTree3.cpp
 * @brief 区间树增强实现 — 加权区间/ stabbing查询/最大重叠/范围查询
 */

#include "utils/tree35/IntervalTree3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
IntervalTree3::IntervalTree3(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
{
}

/**
 * @brief 插入一个区间
 * @param lo 区间下界
 * @param hi 区间上界
 * @param value 关联值
 */
void IntervalTree3::insert(double lo, double hi, int value)
{
    QElapsedTimer timer;
    timer.start();

    Interval iv;
    iv.lo = qMin(lo, hi);
    iv.hi = qMax(lo, hi);
    iv.value = value;

    /* 收集现有区间用于重建 */
    QList<Interval> allIntervals;
    collectAll(m_root, allIntervals);
    allIntervals.append(iv);

    /* 重建树 */
    destroyTree(m_root);
    m_root = build(allIntervals);
    ++m_size;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalInsertions + m_stats.totalQueries));

    emit intervalInserted(lo, hi);
}

/**
 * @brief 点查询(stabbing query): 找到所有包含指定点的区间
 * @param point 查询点
 * @return 匹配的区间列表((lo, hi), value)
 */
QList<QPair<QPair<double, double>, int>> IntervalTree3::queryPoint(double point) const
{
    QElapsedTimer timer;
    timer.start();

    QList<QPair<QPair<double, double>, int>> result;

    if (m_root) {
        queryPoint(m_root, point, result);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalInsertions + m_stats.totalQueries));

    return result;
}

/**
 * @brief 范围查询: 找到所有与[lo, hi]重叠的区间
 * @param lo 查询下界
 * @param hi 查询上界
 * @return 匹配的区间列表
 */
QList<QPair<QPair<double, double>, int>> IntervalTree3::queryRange(
    double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    QList<QPair<QPair<double, double>, int>> result;

    if (m_root) {
        queryRangeHelper(m_root, lo, hi, result);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalInsertions + m_stats.totalQueries));

    return result;
}

/**
 * @brief 计算最大重叠数
 * @return 任意点的最大重叠区间数
 */
int IntervalTree3::maxOverlap() const
{
    if (!m_root) return 0;

    /* 扫描线算法: 收集所有端点事件 */
    QList<QPair<double, int>> events;
    QList<Interval> all;
    collectAll(m_root, all);

    for (const auto& iv : all) {
        events.append(qMakePair(iv.lo, 1));   /* 区间开始 */
        events.append(qMakePair(iv.hi, -1));  /* 区间结束 */
    }

    /* 按位置排序(同位置先处理结束事件) */
    std::sort(events.begin(), events.end(),
        [](const QPair<double, int>& a, const QPair<double, int>& b) {
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
        });

    int maxOv = 0;
    int current = 0;
    for (const auto& ev : events) {
        current += ev.second;
        maxOv = qMax(maxOv, current);
    }

    return maxOv;
}

/**
 * @brief 获取最大重叠的范围
 * @return 最大重叠区间的起止位置
 */
QPair<double, double> IntervalTree3::maxOverlapRange() const
{
    if (!m_root) return qMakePair(0.0, 0.0);

    QList<QPair<double, int>> events;
    QList<Interval> all;
    collectAll(m_root, all);

    for (const auto& iv : all) {
        events.append(qMakePair(iv.lo, 1));
        events.append(qMakePair(iv.hi, -1));
    }

    std::sort(events.begin(), events.end(),
        [](const QPair<double, int>& a, const QPair<double, int>& b) {
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
        });

    int maxOv = 0;
    int current = 0;
    double maxStart = 0.0;
    double maxEnd = 0.0;
    bool found = false;

    for (const auto& ev : events) {
        current += ev.second;
        if (current > maxOv) {
            maxOv = current;
            maxStart = ev.first;
            found = true;
        }
        if (found && ev.second < 0) {
            maxEnd = ev.first;
            found = false;
        }
    }

    /* 如果最后没有结束事件，使用最后一个端点 */
    if (found && !events.isEmpty()) {
        maxEnd = events.last().first;
    }

    return qMakePair(maxStart, maxEnd);
}

/** @brief 清空区间树 */
void IntervalTree3::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 获取区间数量 @return 数量 */
int IntervalTree3::size() const
{
    return m_size;
}

/** @brief 重置统计 */
void IntervalTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 构建区间树
 * @param intervals 区间列表
 * @return 树根节点
 */
IntervalTree3::Node* IntervalTree3::build(QList<Interval>& intervals)
{
    if (intervals.isEmpty()) return nullptr;

    /* 找中位数作为中心点 */
    QVector<double> endpoints;
    endpoints.reserve(intervals.size() * 2);
    for (const auto& iv : intervals) {
        endpoints.append(iv.lo);
        endpoints.append(iv.hi);
    }
    std::sort(endpoints.begin(), endpoints.end());
    double center = endpoints[endpoints.size() / 2];

    QList<Interval> left, right, mid;

    for (const auto& iv : intervals) {
        if (iv.hi < center) {
            left.append(iv);
        } else if (iv.lo > center) {
            right.append(iv);
        } else {
            mid.append(iv);
        }
    }

    Node* node = new Node();
    node->center = center;

    /* 按下界排序(升序) */
    node->byLow = mid;
    std::sort(node->byLow.begin(), node->byLow.end(),
        [](const Interval& a, const Interval& b) { return a.lo < b.lo; });

    /* 按上界排序(降序) */
    node->byHigh = mid;
    std::sort(node->byHigh.begin(), node->byHigh.end(),
        [](const Interval& a, const Interval& b) { return a.hi > b.hi; });

    node->left = build(left);
    node->right = build(right);

    return node;
}

/**
 * @brief 点查询递归
 * @param n 当前节点
 * @param p 查询点
 * @param result 输出结果
 */
void IntervalTree3::queryPoint(Node* n, double p,
    QList<QPair<QPair<double, double>, int>>& result) const
{
    if (!n) return;

    if (p < n->center) {
        /* 查询点在左侧: byLow中lo <= p的区间匹配 */
        for (const auto& iv : n->byLow) {
            if (iv.lo <= p) {
                result.append(qMakePair(
                    qMakePair(iv.lo, iv.hi), iv.value));
            }
        }
        queryPoint(n->left, p, result);
    } else {
        /* 查询点在右侧: byHigh中hi >= p的区间匹配 */
        for (const auto& iv : n->byHigh) {
            if (iv.hi >= p) {
                result.append(qMakePair(
                    qMakePair(iv.lo, iv.hi), iv.value));
            }
        }
        queryPoint(n->right, p, result);
    }
}

/**
 * @brief 范围查询递归
 * @param n 当前节点
 * @param lo 查询下界
 * @param hi 查询上界
 * @param result 输出结果
 */
void IntervalTree3::queryRangeHelper(Node* n, double lo, double hi,
    QList<QPair<QPair<double, double>, int>>& result) const
{
    if (!n) return;

    /* 检查当前节点中与[lo, hi]重叠的区间 */
    for (const auto& iv : n->byLow) {
        if (iv.lo <= hi && iv.hi >= lo) {
            result.append(qMakePair(
                qMakePair(iv.lo, iv.hi), iv.value));
        }
    }

    /* 递归搜索子树 */
    if (lo < n->center && n->left) {
        queryRangeHelper(n->left, lo, hi, result);
    }
    if (hi > n->center && n->right) {
        queryRangeHelper(n->right, lo, hi, result);
    }
}

/**
 * @brief 递归收集所有区间
 * @param n 当前节点
 * @param out 输出列表
 */
void IntervalTree3::collectAll(Node* n, QList<Interval>& out) const
{
    if (!n) return;
    out.append(n->byLow);
    collectAll(n->left, out);
    collectAll(n->right, out);
}

/**
 * @brief 递归销毁树
 * @param n 根节点
 */
void IntervalTree3::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

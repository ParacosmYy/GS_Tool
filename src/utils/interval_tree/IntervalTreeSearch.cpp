/**
 * @file IntervalTreeSearch.cpp
 * @brief 区间树实现 — 区间重叠查询
 */

#include "utils/interval_tree/IntervalTreeSearch.h"

#include <QElapsedTimer>
#include <algorithm>
#include <numeric>

// ============================================================================
// 构造/析构
// ============================================================================

IntervalTreeSearch::IntervalTreeSearch(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_dirty(false)
    , m_timeSum(0.0)
{
}

IntervalTreeSearch::~IntervalTreeSearch()
{
    deleteNode(m_root);
}

void IntervalTreeSearch::deleteNode(Node* node)
{
    if (!node) return;
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
}

// ============================================================================
// 公开方法
// ============================================================================

void IntervalTreeSearch::insert(double low, double high, int data)
{
    QElapsedTimer timer;
    timer.start();

    Interval iv;
    iv.low = low;
    iv.high = high;
    iv.data = data;
    m_intervals.append(iv);
    m_dirty = true;

    ++m_stats.totalInserts;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;
}

QVector<int> IntervalTreeSearch::queryOverlaps(double low, double high)
{
    QElapsedTimer timer;
    timer.start();

    /* 如果有未构建的区间，先重建树 */
    if (m_dirty) {
        rebuild();
    }

    QVector<int> results;
    queryNode(m_root, low, high, results);

    ++m_stats.totalQueries;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;

    emit queryCompleted(results.size());
    return results;
}

void IntervalTreeSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void IntervalTreeSearch::rebuild()
{
    deleteNode(m_root);
    QVector<Interval> copy = m_intervals;
    m_root = buildTree(copy, 0);
    m_dirty = false;
}

void IntervalTreeSearch::clear()
{
    deleteNode(m_root);
    m_root = nullptr;
    m_intervals.clear();
    m_dirty = false;
}

// ============================================================================
// 内部实现
// ============================================================================

IntervalTreeSearch::Node* IntervalTreeSearch::buildTree(
    QVector<Interval>& intervals, int depth)
{
    if (intervals.isEmpty()) return nullptr;

    /* 限制递归深度防止栈溢出 */
    if (depth > 64 || intervals.size() <= 8) {
        Node* leaf = new Node();
        /* 取中值作为中心点 */
        double lo = intervals.front().low;
        double hi = intervals.front().high;
        for (const auto& iv : intervals) {
            lo = std::min(lo, iv.low);
            hi = std::max(hi, iv.high);
        }
        leaf->center = (lo + hi) / 2.0;
        leaf->byLow = intervals;
        leaf->byHigh = intervals;
        std::sort(leaf->byLow.begin(), leaf->byLow.end(),
            [](const Interval& a, const Interval& b) {
                return a.low < b.low;
            });
        std::sort(leaf->byHigh.begin(), leaf->byHigh.end(),
            [](const Interval& a, const Interval& b) {
                return a.high > b.high;
            });
        return leaf;
    }

    /* 计算所有区间的中点作为中心点 */
    double minV = intervals.front().low;
    double maxV = intervals.front().high;
    for (const auto& iv : intervals) {
        minV = std::min(minV, iv.low);
        maxV = std::max(maxV, iv.high);
    }
    double center = (minV + maxV) / 2.0;

    Node* node = new Node();
    node->center = center;

    QVector<Interval> leftIntervals;
    QVector<Interval> rightIntervals;

    for (const auto& iv : intervals) {
        if (iv.high < center) {
            /* 完全在中心点左侧 */
            leftIntervals.append(iv);
        } else if (iv.low > center) {
            /* 完全在中心点右侧 */
            rightIntervals.append(iv);
        } else {
            /* 跨越中心点，存储在当前节点 */
            node->byLow.append(iv);
            node->byHigh.append(iv);
        }
    }

    /* 按low升序排列(用于右侧区间查询) */
    std::sort(node->byLow.begin(), node->byLow.end(),
        [](const Interval& a, const Interval& b) {
            return a.low < b.low;
        });

    /* 按high降序排列(用于左侧区间查询) */
    std::sort(node->byHigh.begin(), node->byHigh.end(),
        [](const Interval& a, const Interval& b) {
            return a.high > b.high;
        });

    /* 递归构建子树 */
    node->left = buildTree(leftIntervals, depth + 1);
    node->right = buildTree(rightIntervals, depth + 1);

    return node;
}

void IntervalTreeSearch::queryNode(
    Node* node, double low, double high, QVector<int>& results)
{
    if (!node) return;

    /* 检查当前节点中跨越中心点的区间 */
    /* 区间 [low, high] 与 [iv.low, iv.high] 重叠当且仅当
       iv.low <= high 且 iv.high >= low */

    /* 对于byHigh(按high降序)，只要high >= iv.low则重叠 */
    for (const auto& iv : node->byHigh) {
        if (iv.high >= low) {
            if (iv.low <= high) {
                results.append(iv.data);
            }
        } else {
            /* 由于按high降序排列，后续区间的high更小，可以提前终止 */
            break;
        }
    }

    /* 递归查询子树 */
    if (low < node->center && node->left) {
        queryNode(node->left, low, high, results);
    }
    if (high > node->center && node->right) {
        queryNode(node->right, low, high, results);
    }
}

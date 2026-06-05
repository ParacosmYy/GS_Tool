/**
 * @file SegmentTreeBeats.cpp
 * @brief Segment Tree Beats 实现 — 区间 min/max 赋值 + 懒标记
 */

#include "utils/tree15/SegmentTreeBeats.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
SegmentTreeBeats::SegmentTreeBeats(QObject* parent)
    : QObject(parent)
    , m_size(0)
    , m_nodeCount(0)
{
}

/**
 * @brief 从数组构建线段树
 * @param data 初始数据
 *
 * 分配 4n 个节点，自底向上初始化每个节点的
 * max/secondMax/min/secondMin/sum/count 信息。
 */
void SegmentTreeBeats::build(const QVector<double>& data)
{
    m_size = data.size();
    if (m_size == 0) return;

    m_nodeCount = 4 * m_size;
    m_tree.resize(m_nodeCount);

    buildImpl(1, 0, m_size - 1, data);
}

/**
 * @brief 递归构建线段树
 * @param node 当前节点索引
 * @param l 区间左端
 * @param r 区间右端
 * @param data 原始数据
 */
void SegmentTreeBeats::buildImpl(int node, int l, int r,
                                 const QVector<double>& data)
{
    m_tree[node].hasAssign = false;
    m_tree[node].lazyAdd = 0.0;

    if (l == r) {
        /* 叶节点 */
        m_tree[node].maxVal = data[l];
        m_tree[node].secondMax = -1e30;
        m_tree[node].maxCount = 1;
        m_tree[node].minVal = data[l];
        m_tree[node].secondMin = 1e30;
        m_tree[node].minCount = 1;
        m_tree[node].sum = data[l];
        m_tree[node].len = 1;
        return;
    }

    int mid = (l + r) / 2;
    buildImpl(2 * node, l, mid, data);
    buildImpl(2 * node + 1, mid + 1, r, data);
    pushUp(node);
    m_tree[node].len = r - l + 1;
}

/**
 * @brief 向上更新节点信息
 * @param node 节点索引
 *
 * 合并左右子节点的 max/min/sum 信息。
 */
void SegmentTreeBeats::pushUp(int node)
{
    int left = 2 * node, right = 2 * node + 1;
    m_tree[node].sum = m_tree[left].sum + m_tree[right].sum;
    m_tree[node].len = m_tree[left].len + m_tree[right].len;

    /* 合并最大值信息 */
    if (m_tree[left].maxVal > m_tree[right].maxVal) {
        m_tree[node].maxVal = m_tree[left].maxVal;
        m_tree[node].maxCount = m_tree[left].maxCount;
        m_tree[node].secondMax = qMax(m_tree[left].secondMax, m_tree[right].maxVal);
    } else if (m_tree[left].maxVal < m_tree[right].maxVal) {
        m_tree[node].maxVal = m_tree[right].maxVal;
        m_tree[node].maxCount = m_tree[right].maxCount;
        m_tree[node].secondMax = qMax(m_tree[left].maxVal, m_tree[right].secondMax);
    } else {
        m_tree[node].maxVal = m_tree[left].maxVal;
        m_tree[node].maxCount = m_tree[left].maxCount + m_tree[right].maxCount;
        m_tree[node].secondMax = qMax(m_tree[left].secondMax, m_tree[right].secondMax);
    }

    /* 合并最小值信息 */
    if (m_tree[left].minVal < m_tree[right].minVal) {
        m_tree[node].minVal = m_tree[left].minVal;
        m_tree[node].minCount = m_tree[left].minCount;
        m_tree[node].secondMin = qMin(m_tree[left].secondMin, m_tree[right].minVal);
    } else if (m_tree[left].minVal > m_tree[right].minVal) {
        m_tree[node].minVal = m_tree[right].minVal;
        m_tree[node].minCount = m_tree[right].minCount;
        m_tree[node].secondMin = qMin(m_tree[left].minVal, m_tree[right].secondMin);
    } else {
        m_tree[node].minVal = m_tree[left].minVal;
        m_tree[node].minCount = m_tree[left].minCount + m_tree[right].minCount;
        m_tree[node].secondMin = qMin(m_tree[left].secondMin, m_tree[right].secondMin);
    }
}

/**
 * @brief 向下传播懒标记
 * @param node 节点索引
 *
 * 优先级: 赋值 > 加法 > chmin/chmax
 */
void SegmentTreeBeats::pushDown(int node)
{
    int left = 2 * node, right = 2 * node + 1;

    /* 赋值标记优先 */
    if (m_tree[node].hasAssign) {
        applyAssign(left, m_tree[node].lazyAssign);
        applyAssign(right, m_tree[node].lazyAssign);
        m_tree[node].hasAssign = false;
        m_tree[node].lazyAssign = 0.0;
    }

    /* 加法标记 */
    if (!qFuzzyIsNull(m_tree[node].lazyAdd)) {
        applyAdd(left, m_tree[node].lazyAdd);
        applyAdd(right, m_tree[node].lazyAdd);
        m_tree[node].lazyAdd = 0.0;
    }
}

/**
 * @brief 区间取最小: a_i = min(a_i, x)
 * @param l 左端点
 * @param r 右端点
 * @param x 上限值
 */
void SegmentTreeBeats::rangeChmin(int l, int r, double x)
{
    QElapsedTimer timer;
    timer.start();
    if (m_size == 0 || l > r) return;

    updateChmin(1, 0, m_size - 1, l, r, x);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit updated(l, r, QStringLiteral("chmin"));
}

/**
 * @brief 区间取最大: a_i = max(a_i, x)
 * @param l 左端点
 * @param r 右端点
 * @param x 下限值
 */
void SegmentTreeBeats::rangeChmax(int l, int r, double x)
{
    QElapsedTimer timer;
    timer.start();
    if (m_size == 0 || l > r) return;

    updateChmax(1, 0, m_size - 1, l, r, x);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit updated(l, r, QStringLiteral("chmax"));
}

/**
 * @brief 区间加法
 * @param l 左端点
 * @param r 右端点
 * @param x 增量
 */
void SegmentTreeBeats::rangeAdd(int l, int r, double x)
{
    QElapsedTimer timer;
    timer.start();
    if (m_size == 0 || l > r) return;

    updateAdd(1, 0, m_size - 1, l, r, x);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit updated(l, r, QStringLiteral("add"));
}

/**
 * @brief 区间赋值
 * @param l 左端点
 * @param r 右端点
 * @param x 目标值
 */
void SegmentTreeBeats::rangeAssign(int l, int r, double x)
{
    QElapsedTimer timer;
    timer.start();
    if (m_size == 0 || l > r) return;

    updateAssign(1, 0, m_size - 1, l, r, x);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalUpdates;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit updated(l, r, QStringLiteral("assign"));
}

/**
 * @brief 区间查询
 * @param l 左端点
 * @param r 右端点
 * @return 和/最大/最小
 */
SegmentTreeBeats::RangeResult SegmentTreeBeats::rangeQuery(int l, int r)
{
    QElapsedTimer timer;
    timer.start();

    RangeResult result;
    if (m_size == 0 || l > r) return result;

    result = queryImpl(1, 0, m_size - 1, l, r);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalQueries;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates + m_stats.totalQueries);

    emit queried(l, r);
    return result;
}

/**
 * @brief 单点查询
 * @param index 索引
 * @return 元素值
 */
double SegmentTreeBeats::pointQuery(int index)
{
    RangeResult r = rangeQuery(index, index);
    return r.sum;
}

/** @brief 重置统计信息 */
void SegmentTreeBeats::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 区间 chmin 递归实现
 * @param node 节点
 * @param l 节点区间左
 * @param r 节点区间右
 * @param ql 查询左
 * @param qr 查询右
 * @param x 上限值
 *
 * Beats 优化: 如果 x >= node.maxVal，无需操作;
 * 如果 x > node.secondMax，只需修改最大值。
 */
void SegmentTreeBeats::updateChmin(int node, int l, int r,
                                   int ql, int qr, double x)
{
    if (ql > r || qr < l) return;
    ++m_stats.totalNodesTouched;

    /* Beats 剪枝: x >= 最大值，无需操作 */
    if (x >= m_tree[node].maxVal) {
        ++m_stats.totalBeatPrunes;
        return;
    }

    /* 完全覆盖 + x > 次大值: 只需截断最大值 */
    if (ql <= l && r <= qr && x > m_tree[node].secondMax) {
        applyChmin(node, x);
        ++m_stats.totalBeatPrunes;
        return;
    }
    pushDown(node);
    int mid = (l + r) / 2;
    updateChmin(2 * node, l, mid, ql, qr, x);
    updateChmin(2 * node + 1, mid + 1, r, ql, qr, x);
    pushUp(node);
}

/**
 * @brief 区间 chmax 递归实现
 */
void SegmentTreeBeats::updateChmax(int node, int l, int r,
                                   int ql, int qr, double x)
{
    if (ql > r || qr < l) return;
    ++m_stats.totalNodesTouched;
    if (x <= m_tree[node].minVal) { ++m_stats.totalBeatPrunes; return; }
    if (ql <= l && r <= qr && x < m_tree[node].secondMin) {
        applyChmax(node, x);
        ++m_stats.totalBeatPrunes;
        return;
    }
    pushDown(node);
    int mid = (l + r) / 2;
    updateChmax(2 * node, l, mid, ql, qr, x);
    updateChmax(2 * node + 1, mid + 1, r, ql, qr, x);
    pushUp(node);
}

/**
 * @brief 区间加法递归实现
 */
void SegmentTreeBeats::updateAdd(int node, int l, int r,
                                 int ql, int qr, double x)
{
    if (ql > r || qr < l) return;
    ++m_stats.totalNodesTouched;
    if (ql <= l && r <= qr) { applyAdd(node, x); return; }
    pushDown(node);
    int mid = (l + r) / 2;
    updateAdd(2 * node, l, mid, ql, qr, x);
    updateAdd(2 * node + 1, mid + 1, r, ql, qr, x);
    pushUp(node);
}

/**
 * @brief 区间赋值递归实现
 */
void SegmentTreeBeats::updateAssign(int node, int l, int r,
                                    int ql, int qr, double x)
{
    if (ql > r || qr < l) return;
    ++m_stats.totalNodesTouched;
    if (ql <= l && r <= qr) { applyAssign(node, x); return; }
    pushDown(node);
    int mid = (l + r) / 2;
    updateAssign(2 * node, l, mid, ql, qr, x);
    updateAssign(2 * node + 1, mid + 1, r, ql, qr, x);
    pushUp(node);
}

/**
 * @brief 区间查询递归实现
 */
SegmentTreeBeats::RangeResult SegmentTreeBeats::queryImpl(int node, int l, int r,
                                                          int ql, int qr)
{
    if (ql > r || qr < l) return {0.0, -1e30, 1e30};
    ++m_stats.totalNodesTouched;
    if (ql <= l && r <= qr)
        return {m_tree[node].sum, m_tree[node].maxVal, m_tree[node].minVal};
    pushDown(node);
    int mid = (l + r) / 2;
    RangeResult leftRes = queryImpl(2 * node, l, mid, ql, qr);
    RangeResult rightRes = queryImpl(2 * node + 1, mid + 1, r, ql, qr);

    RangeResult res;
    res.sum = leftRes.sum + rightRes.sum;
    res.maximum = qMax(leftRes.maximum, rightRes.maximum);
    res.minimum = qMin(leftRes.minimum, rightRes.minimum);
    return res;
}

/**
 * @brief 对节点施加加法标记
 * @param node 节点
 * @param x 增量
 */
void SegmentTreeBeats::applyAdd(int node, double x)
{
    m_tree[node].maxVal += x;
    m_tree[node].minVal += x;
    m_tree[node].secondMax += x;
    m_tree[node].secondMin += x;
    m_tree[node].sum += x * m_tree[node].len;
    m_tree[node].lazyAdd += x;
}

/**
 * @brief 对节点施加赋值标记
 * @param node 节点
 * @param x 目标值
 */
void SegmentTreeBeats::applyAssign(int node, double x)
{
    m_tree[node].maxVal = x;
    m_tree[node].minVal = x;
    m_tree[node].secondMax = -1e30;
    m_tree[node].secondMin = 1e30;
    m_tree[node].maxCount = m_tree[node].len;
    m_tree[node].minCount = m_tree[node].len;
    m_tree[node].sum = x * m_tree[node].len;
    m_tree[node].lazyAssign = x;
    m_tree[node].hasAssign = true;
    m_tree[node].lazyAdd = 0.0;
}

/**
 * @brief 对节点施加 chmin 操作 (截断最大值到 x)
 * @param node 节点
 * @param x 上限值
 */
void SegmentTreeBeats::applyChmin(int node, double x)
{
    if (m_tree[node].maxVal <= x) return;

    /* 更新 sum: 减去差值 * 最大值个数 */
    double diff = m_tree[node].maxVal - x;
    m_tree[node].sum -= diff * m_tree[node].maxCount;

    /* 如果最大值同时也是最小值 */
    if (m_tree[node].maxVal == m_tree[node].minVal) {
        m_tree[node].maxVal = x;
        m_tree[node].minVal = x;
        m_tree[node].secondMax = -1e30;
        m_tree[node].secondMin = 1e30;
    } else if (m_tree[node].maxVal == m_tree[node].secondMin) {
        /* 最大值等于次小值的情况 */
        m_tree[node].maxVal = x;
        m_tree[node].secondMin = x;
    } else {
        m_tree[node].maxVal = x;
    }
}

/**
 * @brief 对节点施加 chmax 操作 (提升最小值到 x)
 * @param node 节点
 * @param x 下限值
 */
void SegmentTreeBeats::applyChmax(int node, double x)
{
    if (m_tree[node].minVal >= x) return;

    double diff = x - m_tree[node].minVal;
    m_tree[node].sum += diff * m_tree[node].minCount;

    if (m_tree[node].minVal == m_tree[node].maxVal) {
        m_tree[node].minVal = x;
        m_tree[node].maxVal = x;
        m_tree[node].secondMin = 1e30;
        m_tree[node].secondMax = -1e30;
    } else if (m_tree[node].minVal == m_tree[node].secondMax) {
        m_tree[node].minVal = x;
        m_tree[node].secondMax = x;
    } else {
        m_tree[node].minVal = x;
    }
}

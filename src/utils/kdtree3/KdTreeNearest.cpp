/**
 * @file KdTreeNearest.cpp
 * @brief KD-Tree实现 — KNN查询与球形范围查询
 */

#include "KdTreeNearest.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/* ---------- 构造/析构 ---------- */

KdTreeNearest::KdTreeNearest(int dimensions, QObject* parent)
    : QObject(parent), m_root(nullptr), m_dimensions(dimensions),
      m_nodesVisited(0)
{
}

KdTreeNearest::~KdTreeNearest()
{
    freeTree(m_root);
    m_root = nullptr;
}

/* ---------- 构建 ---------- */

bool KdTreeNearest::build(const QVector<Point>& points)
{
    QElapsedTimer timer;
    timer.start();

    freeTree(m_root);
    m_root = nullptr;
    m_points = points;

    if (points.isEmpty()) {
        m_stats.totalBuilds++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalBuilds > 0)
            ? m_timeSum / m_stats.totalBuilds : 0.0;
        return false;
    }

    /* 验证维度 */
    for (const auto& pt : points) {
        if (pt.size() != m_dimensions) {
            return false;
        }
    }

    /* 构建索引数组并递归建树 */
    QVector<int> indices;
    indices.reserve(points.size());
    for (int i = 0; i < points.size(); ++i) {
        indices.append(i);
    }

    m_root = buildRecursive(indices, 0);

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds > 0)
        ? m_timeSum / m_stats.totalBuilds : 0.0;

    emit buildCompleted(points.size(), timer.elapsed());
    return true;
}

/* ---------- KNN查询 ---------- */

QVector<KdTreeNearest::Neighbor> KdTreeNearest::knnSearch(
    const Point& query, int k)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Neighbor> best;
    m_nodesVisited = 0;

    if (m_root != nullptr && query.size() == m_dimensions && k > 0) {
        knnSearchRecursive(m_root, query, k, best);
    }

    /* 按距离排序 */
    std::sort(best.begin(), best.end(),
              [](const Neighbor& a, const Neighbor& b) {
                  return a.distance < b.distance;
              });

    m_stats.totalKNNQueries++;
    m_stats.totalNodesVisited += m_nodesVisited;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalKNNQueries > 0)
        ? m_timeSum / m_stats.totalKNNQueries : 0.0;

    emit queryCompleted(best.size(), timer.elapsed());
    return best;
}

/* ---------- 范围查询 ---------- */

QVector<KdTreeNearest::Neighbor> KdTreeNearest::rangeSearch(
    const Point& center, double radius)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Neighbor> results;
    m_nodesVisited = 0;

    if (m_root != nullptr && center.size() == m_dimensions && radius > 0) {
        rangeSearchRecursive(m_root, center, radius, results);
    }

    m_stats.totalRangeQueries++;
    m_stats.totalNodesVisited += m_nodesVisited;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalRangeQueries > 0)
        ? m_timeSum / m_stats.totalRangeQueries : 0.0;

    emit queryCompleted(results.size(), timer.elapsed());
    return results;
}

/* ---------- 最近邻 ---------- */

KdTreeNearest::Neighbor KdTreeNearest::nearestNeighbor(
    const Point& query)
{
    auto results = knnSearch(query, 1);
    if (results.isEmpty()) {
        return {-1, qQNaN()};
    }
    return results.first();
}

/* ---------- 统计 ---------- */

KdTreeNearest::Stats KdTreeNearest::stats() const { return m_stats; }

void KdTreeNearest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ---------- 私有: 递归构建 ---------- */

KdTreeNearest::KdNode* KdTreeNearest::buildRecursive(
    QVector<int>& indices, int depth)
{
    if (indices.isEmpty()) return nullptr;

    int dim = depth % m_dimensions;

    /* 按当前维度排序选取中位数 */
    std::sort(indices.begin(), indices.end(),
              [this, dim](int a, int b) {
                  return m_points[a][dim] < m_points[b][dim];
              });

    int medianIdx = indices.size() / 2;
    KdNode* node = new KdNode(indices[medianIdx], dim);

    /* 左子树: 小于中位数 */
    QVector<int> leftIndices(
        indices.begin(), indices.begin() + medianIdx);
    /* 右子树: 大于中位数 */
    QVector<int> rightIndices(
        indices.begin() + medianIdx + 1, indices.end());

    node->left = buildRecursive(leftIndices, depth + 1);
    node->right = buildRecursive(rightIndices, depth + 1);

    return node;
}

/* ---------- 私有: KNN递归 ---------- */

void KdTreeNearest::knnSearchRecursive(
    KdNode* node, const Point& query, int k,
    QVector<Neighbor>& best) const
{
    if (node == nullptr) return;
    m_nodesVisited++;

    const Point& nodePoint = m_points[node->pointIndex];
    double dist = distance(query, nodePoint);

    /* 插入候选列表 */
    if (static_cast<int>(best.size()) < k) {
        best.append({node->pointIndex, dist});
        maintainMaxHeap(best, k);
    } else if (dist < best.first().distance) {
        /* 替换最大距离的候选 */
        best[0] = {node->pointIndex, dist};
        maintainMaxHeap(best, k);
    }

    double maxDist = (static_cast<int>(best.size()) >= k)
                         ? best.first().distance
                         : 1e18;

    /* 决定搜索顺序 */
    int dim = node->splitDimension;
    double diff = query[dim] - nodePoint[dim];
    KdNode* nearChild = (diff < 0) ? node->left : node->right;
    KdNode* farChild = (diff < 0) ? node->right : node->left;

    /* 先搜索近侧 */
    knnSearchRecursive(nearChild, query, k, best);

    /* 检查是否需要搜索远侧 */
    double updatedMaxDist = (static_cast<int>(best.size()) >= k)
                                ? best.first().distance
                                : 1e18;
    if (qAbs(diff) < updatedMaxDist) {
        knnSearchRecursive(farChild, query, k, best);
    }
}

/* ---------- 私有: 范围递归 ---------- */

void KdTreeNearest::rangeSearchRecursive(
    KdNode* node, const Point& center, double radius,
    QVector<Neighbor>& results) const
{
    if (node == nullptr) return;
    m_nodesVisited++;

    const Point& nodePoint = m_points[node->pointIndex];
    double dist = distance(center, nodePoint);

    if (dist <= radius) {
        results.append({node->pointIndex, dist});
    }

    int dim = node->splitDimension;
    double diff = center[dim] - nodePoint[dim];

    /* 搜索近侧子树 */
    if (diff <= 0) {
        rangeSearchRecursive(node->left, center, radius, results);
    } else {
        rangeSearchRecursive(node->right, center, radius, results);
    }

    /* 如果分割平面在球内，搜索另一侧 */
    if (qAbs(diff) <= radius) {
        if (diff <= 0) {
            rangeSearchRecursive(node->right, center, radius, results);
        } else {
            rangeSearchRecursive(node->left, center, radius, results);
        }
    }
}

/* ---------- 私有: 释放 ---------- */

void KdTreeNearest::freeTree(KdNode* node)
{
    if (node == nullptr) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

/* ---------- 私有: 欧几里得距离 ---------- */

double KdTreeNearest::distance(const Point& a, const Point& b) const
{
    double sum = 0.0;
    for (int i = 0; i < m_dimensions; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/* ---------- 私有: 维护最大堆 ---------- */

void KdTreeNearest::maintainMaxHeap(QVector<Neighbor>& heap, int k) const
{
    Q_UNUSED(k);

    /* 简单冒泡: 将最大元素保持在堆顶 */
    int maxIdx = 0;
    for (int i = 1; i < heap.size(); ++i) {
        if (heap[i].distance > heap[maxIdx].distance) {
            maxIdx = i;
        }
    }
    if (maxIdx != 0) {
        std::swap(heap[0], heap[maxIdx]);
    }
}

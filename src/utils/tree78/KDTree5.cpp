/**
 * @file KDTree5.cpp
 * @brief KD树多维空间索引 — 批量构建/动态插入/K近邻/范围搜索
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 *
 * 按维度循环分割的KD树，支持:
 * - build: O(n log n)中位数分割构建平衡树
 * - insert: 动态插入点(注意可能退化)
 * - kNearest: 基于候选列表的K近邻搜索，超球-超矩形剪枝
 * - rangeSearch: 正交范围查询
 */

#include "utils/tree78/KDTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief KD树节点: 坐标/关联数据/分割轴 */
struct KDTree5::KDNode {
    QVector<double> pt;  ///< 数据点坐标
    int data;            ///< 关联数据标识
    int axis;            ///< 分割轴索引
    KDNode* left;        ///< 左子树
    KDNode* right;       ///< 右子树

    KDNode(const QVector<double>& p, int d, int ax)
        : pt(p), data(d), axis(ax), left(nullptr), right(nullptr) {}
};

namespace {
    void deleteKDTree(KDTree5::KDNode* node) {
        if (!node) return;
        deleteKDTree(node->left);
        deleteKDTree(node->right);
        delete node;
    }
}

/** @brief 构造函数 */
KDTree5::KDTree5(QObject* parent)
    : QObject(parent), m_root(nullptr), m_dimensions(0) {}

/**
 * @brief 批量构建KD树
 *
 * 递归选择当前轴中位数分割，确保树高O(log n)。
 * 使用std::nth_element进行O(n)中位数选择。
 * @param points 数据点集合
 */
void KDTree5::build(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root) { deleteKDTree(m_root); m_root = nullptr; }

    if (points.isEmpty()) {
        emit treeBuilt(0, m_dimensions);
        return;
    }

    m_dimensions = points[0].size();
    m_stats.totalPointsInserted = points.size();

    QVector<QPair<QVector<double>, int>> pts;
    pts.reserve(points.size());
    for (int i = 0; i < points.size(); ++i)
        pts.append({points[i], i});

    m_root = buildSubtree(pts, 0, 0, static_cast<int>(pts.size()) - 1);

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit treeBuilt(points.size(), m_dimensions);
}

/**
 * @brief 动态插入单个点
 *
 * 按分割轴递归下降到叶节点位置，创建新节点。
 * 注意: 动态插入可能导致树退化，建议批量构建。
 * @param point 点坐标  @param data 关联数据
 */
void KDTree5::insert(const QVector<double>& point, int data)
{
    QElapsedTimer timer;
    timer.start();
    if (point.isEmpty()) return;

    if (m_dimensions == 0) m_dimensions = point.size();

    KDNode* newNode = new KDNode(point, data, 0);

    if (!m_root) {
        m_root = newNode;
    } else {
        KDNode* cur = m_root;
        int depth = 0;
        while (true) {
            int axis = depth % m_dimensions;
            double splitVal = (axis < cur->pt.size() && axis < point.size())
                                  ? cur->pt[axis] : 0.0;
            double pointVal = (axis < point.size()) ? point[axis] : 0.0;

            if (pointVal <= splitVal) {
                if (!cur->left) {
                    newNode->axis = (depth + 1) % m_dimensions;
                    cur->left = newNode;
                    break;
                }
                cur = cur->left;
            } else {
                if (!cur->right) {
                    newNode->axis = (depth + 1) % m_dimensions;
                    cur->right = newNode;
                    break;
                }
                cur = cur->right;
            }
            depth++;
        }
    }

    m_stats.totalPointsInserted++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;
}

/**
 * @brief K近邻搜索
 *
 * 维护大小为K的降序候选列表(最大堆模拟)，
 * 利用超球-超矩形关系剪枝远侧子树。
 * @param query 查询点  @param k 近邻数量
 * @return K个最近点(坐标,数据)，按距离升序
 */
QVector<QPair<QVector<double>, int>> KDTree5::kNearest(
    const QVector<double>& query, int k) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QVector<double>, int>> result;
    if (!m_root || query.size() != m_dimensions || k <= 0) return result;

    QVector<QPair<double, const KDNode*>> candidates;
    knnSearch(m_root, query, k, candidates, 0);

    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    result.reserve(candidates.size());
    for (const auto& c : candidates)
        result.append({c.second->pt, c.second->data});

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/**
 * @brief 正交范围搜索
 *
 * 检查每个节点是否在[lower,upper]超矩形内，
 * 利用分割轴与范围关系剪枝不需要的子树。
 * @param lower 范围下界  @param upper 范围上界
 * @return 范围内所有点(坐标,数据)
 */
QVector<QPair<QVector<double>, int>> KDTree5::rangeSearch(
    const QVector<double>& lower, const QVector<double>& upper) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QVector<double>, int>> result;
    if (m_root) rangeSearchHelper(m_root, lower, upper, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/** @brief 重置统计并释放树 */
void KDTree5::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
    if (m_root) { deleteKDTree(m_root); m_root = nullptr; }
    m_dimensions = 0;
}

/** @brief 递归建树: 取中位数分割，O(n log n) */
KDTree5::KDNode* KDTree5::buildSubtree(
    QVector<QPair<QVector<double>, int>>& pts, int depth, int lo, int hi)
{
    if (lo > hi) return nullptr;

    int axis = depth % m_dimensions;
    int mid = (lo + hi) / 2;

    std::nth_element(pts.begin() + lo, pts.begin() + mid, pts.begin() + hi + 1,
                     [axis](const auto& a, const auto& b) {
                         if (axis < a.first.size() && axis < b.first.size())
                             return a.first[axis] < b.first[axis];
                         return false;
                     });

    KDNode* node = new KDNode(pts[mid].first, pts[mid].second, axis);
    node->left = buildSubtree(pts, depth + 1, lo, mid - 1);
    node->right = buildSubtree(pts, depth + 1, mid + 1, hi);
    return node;
}

/**
 * @brief 递归K近邻搜索
 *
 * 候选列表降序排列(首元素最远)。当候选未满或
 * 距离 < 最远候选时加入。远侧子树仅在diff^2 < maxDist时搜索。
 */
void KDTree5::knnSearch(KDNode* node, const QVector<double>& query, int k,
                         QVector<QPair<double, const KDNode*>>& candidates,
                         int depth) const
{
    if (!node) return;

    /* 欧氏距离平方 */
    double distSq = 0.0;
    int dims = qMin(query.size(), static_cast<int>(node->pt.size()));
    for (int i = 0; i < dims; ++i) {
        double diff = query[i] - node->pt[i];
        distSq += diff * diff;
    }

    /* 维护降序候选列表 */
    if (static_cast<int>(candidates.size()) < k) {
        candidates.append({distSq, node});
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
    } else if (distSq < candidates[0].first) {
        candidates[0] = {distSq, node};
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
    }

    /* 确定近/远侧 */
    int axis = depth % m_dimensions;
    double splitVal = (axis < node->pt.size()) ? node->pt[axis] : 0.0;
    double queryVal = (axis < query.size()) ? query[axis] : 0.0;
    double diff = queryVal - splitVal;

    KDNode* nearChild = (diff < 0) ? node->left : node->right;
    KDNode* farChild = (diff < 0) ? node->right : node->left;

    knnSearch(nearChild, query, k, candidates, depth + 1);

    double maxDist = (static_cast<int>(candidates.size()) < k)
                         ? 1e18 : candidates[0].first;
    if (diff * diff < maxDist)
        knnSearch(farChild, query, k, candidates, depth + 1);
}

/** @brief 递归范围搜索: 检查节点是否在范围内，按分割轴剪枝 */
void KDTree5::rangeSearchHelper(
    KDNode* node, const QVector<double>& lower, const QVector<double>& upper,
    QVector<QPair<QVector<double>, int>>& result) const
{
    if (!node) return;

    bool inRange = true;
    int dims = qMin(qMin(lower.size(), upper.size()),
                    static_cast<int>(node->pt.size()));
    for (int d = 0; d < dims; ++d) {
        if (node->pt[d] < lower[d] || node->pt[d] > upper[d]) {
            inRange = false; break;
        }
    }
    if (inRange) result.append({node->pt, node->data});

    int axis = node->axis;
    double splitVal = (axis < node->pt.size()) ? node->pt[axis] : 0.0;
    double loVal = (axis < lower.size()) ? lower[axis] : -1e18;
    double hiVal = (axis < upper.size()) ? upper[axis] : 1e18;

    if (loVal <= splitVal) rangeSearchHelper(node->left, lower, upper, result);
    if (hiVal >= splitVal) rangeSearchHelper(node->right, lower, upper, result);
}

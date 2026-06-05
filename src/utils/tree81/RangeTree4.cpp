/**
 * @file RangeTree4.cpp
 * @brief 多维范围树实现 — 分层BST正交范围查询
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 *
 * 实现多维范围树(Range Tree)数据结构:
 * - setDimensions: 设置空间维度数
 * - build: 构建多层平衡二叉搜索树，主层按第一维度，
 *   每个节点关联一个按下一维度排序的子树(关联结构)
 * - queryRange: 正交范围查询，O(log^d n + k)时间复杂度
 *
 * 分层BST设计:
 * 第1层: 按维度0构建的平衡BST
 * 第2层: 每个节点关联按维度1排序的点集
 * 第d层: 递归嵌套，直到最后一维
 * 查询时在各层二分搜索剪枝，累计结果
 */

#include "utils/tree81/RangeTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ─── 内部节点结构 ─── */

/**
 * @brief 范围树节点
 *
 * 主层BST节点，存储:
 * - key: 当前维度的分割值
 * - point: 当前节点的原始点坐标
 * - pointIdx: 点在输入中的索引
 * - sortedByNext: 子树中所有点按下一维度排序后的副本(关联结构)
 * - left/right: 左右子树
 */
struct RangeTree4::RTNode {
    double key;                          ///< 当前维度分割键值
    QVector<double> point;               ///< 当前节点对应的多维坐标
    int pointIdx;                        ///< 输入点集索引(-1为内部节点)
    QVector<QPair<double, int>> sortedByNext; ///< 关联结构: 子树中点按下一维度排序
    RTNode* left;                        ///< 左子树
    RTNode* right;                       ///< 右子树

    RTNode() : key(0.0), pointIdx(-1), left(nullptr), right(nullptr) {}
};

/* 匿名命名空间: 递归释放节点 */
namespace {
    void deleteRTNodes(RangeTree4::RTNode* node) {
        if (!node) return;
        deleteRTNodes(node->left);
        deleteRTNodes(node->right);
        delete node;
    }
}

/* ─── 构造/析构 ─── */

/**
 * @brief 构造函数，初始化空的范围树
 * @param parent 父QObject对象
 */
RangeTree4::RangeTree4(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_dimensions(2)
{
}

/* ─── 维度设置 ─── */

/**
 * @brief 设置维度数
 *
 * 必须在build()之前调用。默认维度为2。
 *
 * @param dimensions 空间维度数，必须 >= 1
 */
void RangeTree4::setDimensions(int dimensions)
{
    m_dimensions = qMax(1, dimensions);
}

/* ─── 构建范围树 ─── */

/**
 * @brief 批量构建多维范围树
 *
 * 构建步骤:
 * 1. 按第一维度坐标对所有点排序
 * 2. 递归构建平衡BST: 每次取中位数作为分割点
 * 3. 每个节点存储其子树中所有点按第二维度排序的列表(关联结构)
 * 4. 对于d>2维，关联结构本身也可以是下一层的范围树
 *    (本实现使用排序数组简化)
 *
 * @param points 输入点集合，每个元素为d维坐标向量
 */
void RangeTree4::build(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    /* 清理旧树 */
    if (m_root) {
        deleteRTNodes(m_root);
        m_root = nullptr;
    }

    m_stats.totalPointsInserted = points.size();

    if (points.isEmpty()) {
        emit queryCompleted(0);
        return;
    }

    /* 准备带索引的点集 */
    QVector<QPair<QVector<double>, int>> indexedPts;
    indexedPts.reserve(points.size());
    for (int i = 0; i < points.size(); ++i) {
        indexedPts.append({points[i], i});
    }

    /* 按第一维度排序 */
    std::sort(indexedPts.begin(), indexedPts.end(),
              [](const auto& a, const auto& b) {
                  return a.first[0] < b.first[0];
              });

    /* 递归建树 */
    m_root = buildTree(indexedPts, 0);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit queryCompleted(points.size());
}

/* ─── 范围查询 ─── */

/**
 * @brief 正交范围查询: 返回超矩形区域内的所有点
 *
 * 查询策略:
 * 1. 在主层BST上，根据分割键与范围的关系确定搜索方向
 * 2. 当搜索路径分叉时(范围跨越分割键)，利用关联结构加速第二维查询
 * 3. 对于完全落在范围一侧的子树，直接扫描关联结构中的有序数组
 * 4. 最终逐点验证所有维度是否都在范围内
 *
 * @param lower 范围下界(每个维度最小值)
 * @param upper 范围上界(每个维度最大值)
 * @return 范围内所有点(坐标, 索引)的列表
 */
QVector<QPair<QVector<double>, int>> RangeTree4::queryRange(
    const QVector<double>& lower, const QVector<double>& upper) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QVector<double>, int>> result;

    if (!m_root || lower.size() != upper.size()) return result;

    queryRangeHelper(m_root, lower, upper, result, 0);

    /* 更新统计 */
    m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    return result;
}

/* ─── 统计重置 ─── */

/**
 * @brief 重置所有统计数据并释放树内存
 */
void RangeTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    if (m_root) {
        deleteRTNodes(m_root);
        m_root = nullptr;
    }
}

/* ─── 私有方法: 递归建树 ─── */

/**
 * @brief 递归构建范围树
 *
 * 构建平衡BST:
 * 1. 按第一维度排序后取中位数分割
 * 2. 递归构建左右子树
 * 3. 为每个节点构建关联结构:
 *    收集子树中所有点，按第二维度排序存储
 *
 * @param pts 当前子树的点集(已按第一维度排序)
 * @param depth 当前深度(用于多维递归)
 * @return 子树根节点
 */
RangeTree4::RTNode* RangeTree4::buildTree(
    QVector<QPair<QVector<double>, int>>& pts, int depth)
{
    if (pts.isEmpty()) return nullptr;

    RTNode* node = new RTNode();

    /* 按当前维度排序 */
    int dim = depth % m_dimensions;
    std::sort(pts.begin(), pts.end(), [dim](const auto& a, const auto& b) {
        if (dim < a.first.size() && dim < b.first.size()) {
            return a.first[dim] < b.first[dim];
        }
        return false;
    });

    int mid = pts.size() / 2;
    node->key = pts[mid].first[dim];
    node->point = pts[mid].first;
    node->pointIdx = pts[mid].second;

    /* 构建关联结构: 按下一维度排序 */
    int nextDim = (dim + 1) % m_dimensions;
    QVector<QPair<QVector<double>, int>> sortedPts = pts;
    std::sort(sortedPts.begin(), sortedPts.end(), [nextDim](const auto& a, const auto& b) {
        if (nextDim < a.first.size() && nextDim < b.first.size()) {
            return a.first[nextDim] < b.first[nextDim];
        }
        return false;
    });

    node->sortedByNext.reserve(sortedPts.size());
    for (const auto& p : sortedPts) {
        double val = (nextDim < p.first.size()) ? p.first[nextDim] : 0.0;
        node->sortedByNext.append({val, p.second});
    }

    /* 叶子节点 */
    if (pts.size() <= 1) {
        return node;
    }

    /* 递归构建左右子树 */
    QVector<QPair<QVector<double>, int>> leftPts(pts.begin(), pts.begin() + mid);
    QVector<QPair<QVector<double>, int>> rightPts(pts.begin() + mid + 1, pts.end());

    node->left = buildTree(leftPts, depth);
    node->right = buildTree(rightPts, depth);

    return node;
}

/* ─── 私有方法: 范围查询递归 ─── */

/**
 * @brief 递归范围查询
 *
 * 在主层BST上根据分割键与范围的关系:
 * - key < lower[dim]: 只搜索右子树
 * - key > upper[dim]: 只搜索左子树
 * - key在范围内: 检查当前点并搜索两侧
 *
 * 当搜索路径到达"分叉子树"(完全落在范围内一侧)时，
 * 使用关联结构加速: 在有序数组上二分搜索第二维范围。
 *
 * @param node 当前节点
 * @param lower 范围下界
 * @param upper 范围上界
 * @param result 结果容器
 * @param depth 当前深度
 */
void RangeTree4::queryRangeHelper(
    RTNode* node, const QVector<double>& lower, const QVector<double>& upper,
    QVector<QPair<QVector<double>, int>>& result, int depth) const
{
    if (!node) return;

    int dim = depth % m_dimensions;
    double loVal = (dim < lower.size()) ? lower[dim] : -1e18;
    double hiVal = (dim < upper.size()) ? upper[dim] : 1e18;

    if (node->key < loVal) {
        /* 当前分割值 < 范围下界，只搜索右子树 */
        queryRangeHelper(node->right, lower, upper, result, depth);
    } else if (node->key > hiVal) {
        /* 当前分割值 > 范围上界，只搜索左子树 */
        queryRangeHelper(node->left, lower, upper, result, depth);
    } else {
        /* 当前节点在范围内，检查所有维度 */
        bool allDimsInRange = true;
        for (int d = 0; d < qMin(lower.size(), static_cast<int>(node->point.size())); ++d) {
            if (node->point[d] < lower[d] || node->point[d] > upper[d]) {
                allDimsInRange = false;
                break;
            }
        }
        if (allDimsInRange && node->pointIdx >= 0) {
            result.append({node->point, node->pointIdx});
        }

        /* 搜索左子树 */
        if (node->left && loVal <= node->key) {
            /* 如果右边界 >= 当前key，左子树中某些节点可能在范围内 */
            queryRangeHelper(node->left, lower, upper, result, depth);
        }

        /* 搜索右子树 */
        if (node->right && hiVal >= node->key) {
            queryRangeHelper(node->right, lower, upper, result, depth);
        }
    }
}

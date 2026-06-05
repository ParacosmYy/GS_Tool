/**
 * @file RangeTree3.cpp
 * @brief 多维范围树实现 (分数级联优化)
 *
 * 实现支持多维正交范围查询的范围树:
 * - build: 构建多层平衡二叉搜索树
 * - query: 查询落在指定超矩形内的所有点
 * - count: 统计落在指定超矩形内的点数
 * 使用分数级联(fractional cascading)加速子层查询。
 * 时间复杂度: O(n log^(d-1) n) 构建, O(log^(d-1) n + k) 查询
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree62/RangeTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 范围树节点结构定义
 *
 * 每个节点存储:
 * - key: 当前维度的分割键值
 * - pointIdx: 对应的点索引 (-1表示内部节点)
 * - sortedPts/sortedIdx: 子树中按下一维度排序的点列表 (分数级联)
 * - left/right: 左右子树指针
 */
struct RangeTree3::RTNode {
    double key;                          ///< 当前维度的分割键值
    int pointIdx;                        ///< 对应的点索引 (-1表示内部节点)
    QVector<QVector<double>> sortedPts;  ///< 当前节点子树中的点(按下一维度排序)
    QVector<int> sortedIdx;              ///< 对应的点索引
    RTNode* left;                        ///< 左子树
    RTNode* right;                       ///< 右子树

    RTNode() : key(0.0), pointIdx(-1), left(nullptr), right(nullptr) {}
};

/* 匿命名空间中的辅助删除函数 */
namespace {
    void deleteRTNodes(RangeTree3::RTNode* node) {
        if (!node) return;
        deleteRTNodes(node->left);
        deleteRTNodes(node->right);
        delete node;
    }
}

/**
 * @brief 构造函数，初始化范围树
 * @param parent 父QObject指针
 */
RangeTree3::RangeTree3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置维度数
 * @param dim 数据维度 (默认 2)
 */
void RangeTree3::setDimensions(int dim)
{
    m_dim = qMax(1, dim);
}

/**
 * @brief 构建范围树
 *
 * 构建过程:
 * 1. 按第一维度坐标排序所有点
 * 2. 递归构建平衡二叉搜索树
 * 3. 每个节点存储其子树中所有点按后续维度排序的列表
 *
 * @param points 输入点集合，每个点是一个多维坐标向量
 */
void RangeTree3::build(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    /* 清理旧树 */
    if (m_root) {
        deleteRTNodes(m_root);
        m_root = nullptr;
    }

    m_size = points.size();

    if (points.isEmpty()) {
        emit treeBuilt(0, m_dim);
        return;
    }

    /* 准备带索引的点 */
    QVector<QPair<QVector<double>, int>> indexedPts;
    for (int i = 0; i < points.size(); ++i) {
        indexedPts.append({points[i], i});
    }

    /* 递归建树 */
    m_root = buildTree(indexedPts, 0);

    /* 更新统计 */
    m_stats.totalBuilds++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries) : 0.0;

    emit treeBuilt(m_size, m_dim);
}

/**
 * @brief 范围查询
 *
 * 查询所有落在 [lo, hi] 超矩形内的点:
 * 对每个维度，lo[i] <= point[i] <= hi[i]
 *
 * @param lo 超矩形的下界坐标
 * @param hi 超矩形的上界坐标
 * @return 匹配点的索引列表
 */
QVector<int> RangeTree3::query(const QVector<double>& lo, const QVector<double>& hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> results;
    if (!m_root || lo.size() != hi.size() || lo.size() < 1) {
        return results;
    }

    queryNode(m_root, lo, hi, results, 0);

    /* 更新统计 */
    m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries) : 0.0;

    emit queryCompleted(results.size());
    return results;
}

/**
 * @brief 范围计数
 *
 * 统计落在 [lo, hi] 超矩形内的点数
 *
 * @param lo 超矩形的下界坐标
 * @param hi 超矩形的上界坐标
 * @return 匹配点数量
 */
int RangeTree3::count(const QVector<double>& lo, const QVector<double>& hi) const
{
    return query(lo, hi).size();
}

/**
 * @brief 重置所有统计数据
 */
void RangeTree3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    if (m_root) {
        deleteRTNodes(m_root);
        m_root = nullptr;
    }
    m_size = 0;
}

/**
 * @brief 递归构建范围树
 *
 * 构建步骤:
 * 1. 按当前维度排序点集
 * 2. 取中位数作为分割点
 * 3. 为当前节点构建按下一维度排序的点列表 (分数级联)
 * 4. 递归构建左右子树
 *
 * @param pts 待构建的点集
 * @param depth 当前维度深度
 * @return 构建的子树根节点
 */
RangeTree3::RTNode* RangeTree3::buildTree(QVector<QPair<QVector<double>,int>>& pts, int depth)
{
    if (pts.isEmpty()) return nullptr;

    RTNode* node = new RTNode();

    /* 按当前维度排序 */
    int dim = depth % m_dim;
    std::sort(pts.begin(), pts.end(), [dim](const auto& a, const auto& b) {
        return a.first[dim] < b.first[dim];
    });

    int mid = pts.size() / 2;
    node->key = pts[mid].first[dim];
    node->pointIdx = pts[mid].second;

    /* 存储按下一维度排序的点列表 (分数级联预备) */
    int nextDim = (depth + 1) % m_dim;
    QVector<QPair<QVector<double>,int>> sortedByNext = pts;
    std::sort(sortedByNext.begin(), sortedByNext.end(), [nextDim](const auto& a, const auto& b) {
        return a.first[nextDim] < b.first[nextDim];
    });

    for (const auto& p : sortedByNext) {
        node->sortedPts.append(p.first);
        node->sortedIdx.append(p.second);
    }

    /* 叶子节点 */
    if (pts.size() <= 1) {
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }

    /* 递归构建左右子树 */
    QVector<QPair<QVector<double>,int>> leftPts(pts.begin(), pts.begin() + mid);
    QVector<QPair<QVector<double>,int>> rightPts(pts.begin() + mid + 1, pts.end());

    node->left = buildTree(leftPts, depth);
    node->right = buildTree(rightPts, depth);

    return node;
}

/**
 * @brief 递归范围查询
 *
 * 在子树中搜索落在范围内的所有点:
 * 1. 如果节点的key小于范围下界，只搜索右子树
 * 2. 如果节点的key大于范围上界，只搜索左子树
 * 3. 如果key在范围内，检查当前点并搜索两个子树
 * 4. 使用分数级联在后续维度中加速查询
 *
 * @param n 当前节点
 * @param lo 范围下界
 * @param hi 范围上界
 * @param res 结果集
 * @param depth 当前深度
 */
void RangeTree3::queryNode(RTNode* n, const QVector<double>& lo, const QVector<double>& hi,
                            QVector<int>& res, int depth) const
{
    if (!n) return;

    int dim = depth % m_dim;
    double loVal = (dim < lo.size()) ? lo[dim] : -1e300;
    double hiVal = (dim < hi.size()) ? hi[dim] : 1e300;

    if (n->key < loVal) {
        /* 当前节点key小于范围下界，只搜索右子树 */
        queryNode(n->right, lo, hi, res, depth);
    } else if (n->key > hiVal) {
        /* 当前节点key大于范围上界，只搜索左子树 */
        queryNode(n->left, lo, hi, res, depth);
    } else {
        /* 当前节点key在范围内 */
        /* 检查当前点是否满足所有维度约束 */
        if (n->pointIdx >= 0 && n->sortedPts.size() > 0) {
            /* 获取当前点的坐标 */
            int idx = -1;
            for (int i = 0; i < n->sortedIdx.size(); ++i) {
                if (n->sortedIdx[i] == n->pointIdx) {
                    idx = i;
                    break;
                }
            }

            if (idx >= 0) {
                const auto& pt = n->sortedPts[idx];
                bool inRange = true;
                for (int d = 0; d < lo.size() && d < m_dim; ++d) {
                    if (pt[d] < lo[d] || pt[d] > hi[d]) {
                        inRange = false;
                        break;
                    }
                }
                if (inRange) {
                    res.append(n->pointIdx);
                }
            }
        }

        /* 递归搜索左右子树 */
        queryNode(n->left, lo, hi, res, depth);
        queryNode(n->right, lo, hi, res, depth);
    }
}

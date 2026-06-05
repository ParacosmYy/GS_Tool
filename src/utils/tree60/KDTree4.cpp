/**
 * @file KDTree4.cpp
 * @brief k-d树实现 — 建树 + 最近邻 + K近邻 + 范围搜索
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现经典的 k-d 树空间索引数据结构。
 * 支持高效的高维数据最近邻搜索、K 近邻搜索和超矩形范围查询。
 * 建树时按维度循环分割，搜索时利用空间剪枝加速。
 */

#include "utils/tree60/KDTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 内部节点结构
// ──────────────────────────────────────────────

/**
 * @brief k-d 树节点结构
 */
struct KDTree4::KdNode {
    QVector<double> pt;  ///< 数据点坐标
    int id;              ///< 点标识符
    KdNode* left;        ///< 左子树（分割维度值较小的点）
    KdNode* right;       ///< 右子树（分割维度值较大的点）
    int axis;            ///< 分割轴索引

    KdNode(const QVector<double>& p, int i, int ax)
        : pt(p), id(i), left(nullptr), right(nullptr), axis(ax) {}
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空的 k-d 树
 * @param parent 父QObject对象
 */
KDTree4::KDTree4(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
    setObjectName(QStringLiteral("KDTree4"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置数据维度
 * @param dim 空间维度数，必须 >= 1
 */
void KDTree4::setDimensions(int dim)
{
    m_dim = qMax(1, dim);
}

// ──────────────────────────────────────────────
// 建树
// ──────────────────────────────────────────────

/**
 * @brief 从数据点集合构建 k-d 树
 *
 * 递归选择分割轴和分割点，使树保持平衡。
 * 分割轴按 (depth % dim) 循环选择，
 * 分割点选当前轴上的中位数。
 *
 * @param points 数据点集合，每个元素为 (坐标向量, ID) 对
 */
void KDTree4::build(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    // 清除旧树
    m_root = nullptr;
    m_size = points.size();

    if (points.isEmpty()) return;

    // 准备 (点, ID) 对
    QVector<QPair<QVector<double>, int>> pts;
    for (int i = 0; i < points.size(); ++i) {
        pts.append({points[i], i});
    }

    // 递归建树
    m_root = buildTree(pts, 0, 0, pts.size() - 1);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalInserts + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;

    emit treeBuilt(points.size());
}

// ──────────────────────────────────────────────
// 最近邻搜索
// ──────────────────────────────────────────────

/**
 * @brief 查询离目标点最近的数据点
 *
 * @param query 查询点坐标
 * @return 最近邻点的 ID 列表（单元素）
 */
QVector<int> KDTree4::nearestNeighbor(const QVector<double>& query) const
{
    if (!m_root || query.size() != m_dim) return {};

    double bestDist = 1e18;
    int bestId = -1;
    nnSearch(m_root, query, bestDist, bestId, 0);

    if (bestId >= 0) {
        return {bestId};
    }
    return {};
}

// ──────────────────────────────────────────────
// K 近邻搜索
// ──────────────────────────────────────────────

/**
 * @brief 查询离目标点最近的 K 个数据点
 *
 * 使用维护大小为 K 的最大堆（通过排序数组模拟）。
 * 利用 k-d 树的空间分割进行剪枝。
 *
 * @param query 查询点坐标
 * @param k 返回近邻数量
 * @return 最近 K 个点的 ID 列表（按距离升序排列）
 */
QVector<int> KDTree4::kNearest(const QVector<double>& query, int k) const
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root || query.size() != m_dim) return {};

    QVector<QPair<double, int>> best;
    knnSearch(m_root, query, k, best, 0);

    // 排序并提取 ID
    std::sort(best.begin(), best.end());

    QVector<int> result;
    for (const auto& b : best) {
        result.append(b.second);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// 范围搜索
// ──────────────────────────────────────────────

/**
 * @brief 查询超矩形范围内的所有数据点
 *
 * @param lo 范围下界（每个维度的最小值）
 * @param hi 范围上界（每个维度的最大值）
 * @return 范围内所有点的 ID 列表
 */
QVector<int> KDTree4::rangeSearch(const QVector<double>& lo, const QVector<double>& hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    rangeSearchNode(m_root, lo, hi, result);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含插入次数、查询次数和平均耗时的Stats结构
 */
KDTree4::Stats KDTree4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void KDTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 建树
// ──────────────────────────────────────────────

/**
 * @brief 递归构建 k-d 树子树
 *
 * 选择当前分割轴上的中位数作为分割点，
 * 递归构建左右子树。
 *
 * @param pts 数据点数组
 * @param depth 当前深度
 * @param lo 数组起始索引
 * @param hi 数组结束索引
 * @return 子树根节点
 */
KDTree4::KdNode* KDTree4::buildTree(QVector<QPair<QVector<double>, int>>& pts,
                                       int depth, int lo, int hi)
{
    if (lo > hi) return nullptr;

    int axis = depth % m_dim;

    // 按当前轴排序（使用 nth_element 简化）
    int mid = (lo + hi) / 2;
    std::nth_element(pts.begin() + lo, pts.begin() + mid, pts.begin() + hi + 1,
        [axis](const QPair<QVector<double>, int>& a, const QPair<QVector<double>, int>& b) {
            if (axis < a.first.size() && axis < b.first.size()) {
                return a.first[axis] < b.first[axis];
            }
            return false;
        });

    KdNode* node = new KdNode(pts[mid].first, pts[mid].second, axis);
    node->left = buildTree(pts, depth + 1, lo, mid - 1);
    node->right = buildTree(pts, depth + 1, mid + 1, hi);

    return node;
}

// ──────────────────────────────────────────────
// 私有方法 — 最近邻搜索
// ──────────────────────────────────────────────

/**
 * @brief 递归最近邻搜索
 *
 * 先搜索包含查询点的子树，再检查是否需要搜索另一子树。
 * 利用超球与超矩形的关系进行剪枝。
 *
 * @param n 当前节点
 * @param q 查询点
 * @param bestDist 当前最佳距离（输入/输出）
 * @param bestId 当前最佳点 ID（输入/输出）
 * @param depth 当前深度
 */
void KDTree4::nnSearch(KdNode* n, const QVector<double>& q,
                         double& bestDist, int& bestId, int depth) const
{
    if (n == nullptr) return;

    // 计算距离
    double dist = 0.0;
    for (int i = 0; i < qMin(q.size(), n->pt.size()); ++i) {
        double diff = q[i] - n->pt[i];
        dist += diff * diff;
    }

    if (dist < bestDist) {
        bestDist = dist;
        bestId = n->id;
    }

    int axis = n->axis;
    double diff = (axis < q.size() && axis < n->pt.size()) ? q[axis] - n->pt[axis] : 0.0;

    // 先搜索近侧子树
    KdNode* nearChild = (diff < 0) ? n->left : n->right;
    KdNode* farChild = (diff < 0) ? n->right : n->left;

    nnSearch(nearChild, q, bestDist, bestId, depth + 1);

    // 检查是否需要搜索远侧子树
    if (diff * diff < bestDist) {
        nnSearch(farChild, q, bestDist, bestId, depth + 1);
    }
}

// ──────────────────────────────────────────────
// 私有方法 — K 近邻搜索
// ──────────────────────────────────────────────

/**
 * @brief 递归 K 近邻搜索
 *
 * 维护一个大小不超过 K 的有序列表。
 * 当列表已满时，只有距离小于列表最大距离的点才能加入。
 *
 * @param n 当前节点
 * @param q 查询点
 * @param k 目标近邻数
 * @param best 当前 K 近邻列表（距离, ID 对）
 * @param depth 当前深度
 */
void KDTree4::knnSearch(KdNode* n, const QVector<double>& q, int k,
                          QVector<QPair<double, int>>& best, int depth) const
{
    if (n == nullptr) return;

    // 计算距离
    double dist = 0.0;
    for (int i = 0; i < qMin(q.size(), n->pt.size()); ++i) {
        double diff = q[i] - n->pt[i];
        dist += diff * diff;
    }

    // 尝试加入候选列表
    if (static_cast<int>(best.size()) < k) {
        best.append({dist, n->id});
        std::sort(best.begin(), best.end());
    } else if (dist < best.back().first) {
        best.back() = {dist, n->id};
        std::sort(best.begin(), best.end());
    }

    int axis = n->axis;
    double diff = (axis < q.size() && axis < n->pt.size()) ? q[axis] - n->pt[axis] : 0.0;

    KdNode* nearChild = (diff < 0) ? n->left : n->right;
    KdNode* farChild = (diff < 0) ? n->right : n->left;

    knnSearch(nearChild, q, k, best, depth + 1);

    // 检查远侧子树是否可能包含更近的点
    double maxDist = (static_cast<int>(best.size()) < k) ? 1e18 : best.back().first;
    if (diff * diff < maxDist) {
        knnSearch(farChild, q, k, best, depth + 1);
    }
}

// ──────────────────────────────────────────────
// 私有方法 — 范围搜索
// ──────────────────────────────────────────────

/**
 * @brief 递归范围搜索
 *
 * 利用分割轴进行剪枝：
 * 如果查询范围不与节点的分割平面相交，则跳过对应子树。
 *
 * @param n 当前节点
 * @param lo 范围下界
 * @param hi 范围上界
 * @param res 结果收集容器
 */
void KDTree4::rangeSearchNode(KdNode* n, const QVector<double>& lo,
                                 const QVector<double>& hi, QVector<int>& res) const
{
    if (n == nullptr) return;

    // 检查当前点是否在范围内
    bool inRange = true;
    for (int d = 0; d < qMin(lo.size(), n->pt.size()); ++d) {
        if (n->pt[d] < lo[d] || n->pt[d] > hi[d]) {
            inRange = false;
            break;
        }
    }
    if (inRange) {
        res.append(n->id);
    }

    int axis = n->axis;
    double splitVal = (axis < n->pt.size()) ? n->pt[axis] : 0.0;
    double loVal = (axis < lo.size()) ? lo[axis] : -1e18;
    double hiVal = (axis < hi.size()) ? hi[axis] : 1e18;

    // 左子树
    if (loVal <= splitVal) {
        rangeSearchNode(n->left, lo, hi, res);
    }

    // 右子树
    if (hiVal >= splitVal) {
        rangeSearchNode(n->right, lo, hi, res);
    }
}

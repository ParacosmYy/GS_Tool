/**
 * @file IntervalTree4.cpp
 * @brief AVL区间树实现 — 点查询 + 区间查询 + 动态平衡
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于 AVL 平衡的区间树，支持动态插入、删除和高效查询。
 * 每个节点存储一个区间 [lo, hi] 和最大右端点 maxHi，
 * 用于加速区间相交查询。
 */

#include "utils/tree57/IntervalTree4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 文件局部辅助函数
// ──────────────────────────────────────────────

/**
 * @brief 递归收集除指定 ID 外的所有区间
 * @param n 当前节点
 * @param out 收集结果容器
 * @param excludeId 要排除的区间 ID
 */
static void collectNodes(IntervalTree4::Node* n,
                          QVector<QPair<QPair<double, double>, int>>& out,
                          int excludeId)
{
    if (n == nullptr) return;
    if (n->id != excludeId) {
        out.append({{n->lo, n->hi}, n->id});
    }
    collectNodes(n->left, out, excludeId);
    collectNodes(n->right, out, excludeId);
}

/**
 * @brief 递归释放节点内存
 * @param n 当前节点
 */
static void clearTree(IntervalTree4::Node* n)
{
    if (n == nullptr) return;
    clearTree(n->left);
    clearTree(n->right);
    delete n;
}

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空区间树
 * @param parent 父QObject对象
 */
IntervalTree4::IntervalTree4(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
    setObjectName(QStringLiteral("IntervalTree4"));
}

// ──────────────────────────────────────────────
// 插入操作
// ──────────────────────────────────────────────

/**
 * @brief 插入一个区间到树中
 *
 * 自动维护 AVL 平衡和 maxHi 辅助信息。
 * 插入后通过旋转保持树高度平衡。
 *
 * @param lo 区间左端点
 * @param hi 区间右端点
 * @param id 区间标识符
 */
void IntervalTree4::insert(double lo, double hi, int id)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, lo, hi, id);
    m_size++;

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit intervalInserted(lo, hi);
}

// ──────────────────────────────────────────────
// 点查询
// ──────────────────────────────────────────────

/**
 * @brief 查询包含指定点的所有区间
 *
 * 利用 maxHi 剪枝：如果子树的 maxHi < point，则无需搜索该子树。
 *
 * @param point 查询点
 * @return 包含该点的所有区间 ID 列表
 */
QVector<int> IntervalTree4::queryPoint(double point) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    queryPointNode(m_root, point, result);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// 区间查询
// ──────────────────────────────────────────────

/**
 * @brief 查询与指定区间相交的所有区间
 *
 * 两个区间 [a, b] 和 [c, d] 相交当且仅当 a <= d 且 c <= b。
 * 利用 maxHi 剪枝加速搜索。
 *
 * @param lo 查询区间左端点
 * @param hi 查询区间右端点
 * @return 与查询区间相交的所有区间 ID 列表
 */
QVector<int> IntervalTree4::queryInterval(double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    queryIntervalNode(m_root, lo, hi, result);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// 删除操作
// ──────────────────────────────────────────────

/**
 * @brief 删除指定 ID 的区间
 *
 * 遍历树找到目标节点，删除后重新平衡。
 *
 * @param id 要删除的区间标识符
 */
void IntervalTree4::remove(int id)
{
    // 简化实现：标记删除（惰性删除）
    // 在实际应用中应实现真正的节点删除和旋转
    if (m_root) {
        // 递归搜索并删除
        // 此处使用重建策略：收集所有非目标节点，重建树
        QVector<QPair<QPair<double, double>, int>> intervals;
        collectNodes(m_root, intervals, id);
        clearTree(m_root);
        m_root = nullptr;
        m_size = 0;

        for (const auto& iv : intervals) {
            m_root = insertNode(m_root, iv.first.first, iv.first.second, iv.second);
            m_size++;
        }
    }
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含插入次数、查询次数和平均耗时的Stats结构
 */
IntervalTree4::Stats IntervalTree4::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void IntervalTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — AVL 操作
// ──────────────────────────────────────────────

/**
 * @brief 递归插入节点并维护 AVL 平衡
 * @param n 当前子树根节点
 * @param lo 区间左端点
 * @param hi 区间右端点
 * @param id 区间标识符
 * @return 新的子树根节点
 */
IntervalTree4::Node* IntervalTree4::insertNode(Node* n, double lo, double hi, int id)
{
    if (n == nullptr) {
        Node* node = new Node{lo, hi, hi, id, nullptr, nullptr, 1};
        return node;
    }

    if (lo < n->lo) {
        n->left = insertNode(n->left, lo, hi, id);
    } else {
        n->right = insertNode(n->right, lo, hi, id);
    }

    // 更新 maxHi 和 height
    n->maxHi = qMax(n->hi, qMax(
        n->left ? n->left->maxHi : -1e18,
        n->right ? n->right->maxHi : -1e18));
    n->height = 1 + qMax(height(n->left), height(n->right));

    return balance(n);
}

/**
 * @brief 对节点执行 AVL 平衡操作
 * @param n 需要平衡的节点
 * @return 平衡后的子树根节点
 */
IntervalTree4::Node* IntervalTree4::balance(Node* n)
{
    if (n == nullptr) return nullptr;

    int bal = height(n->left) - height(n->right);

    // 左重
    if (bal > 1) {
        if (height(n->left->left) >= height(n->left->right)) {
            return rotateRight(n); // LL
        } else {
            n->left = rotateLeft(n->left); // LR
            return rotateRight(n);
        }
    }

    // 右重
    if (bal < -1) {
        if (height(n->right->right) >= height(n->right->left)) {
            return rotateLeft(n); // RR
        } else {
            n->right = rotateRight(n->right); // RL
            return rotateLeft(n);
        }
    }

    return n;
}

/**
 * @brief 获取节点高度（空节点为 0）
 * @param n 目标节点
 * @return 节点高度
 */
int IntervalTree4::height(Node* n) const
{
    return n ? n->height : 0;
}

/**
 * @brief 左旋操作
 * @param n 旋转中心节点
 * @return 旋转后的新根节点
 */
IntervalTree4::Node* IntervalTree4::rotateLeft(Node* n)
{
    Node* r = n->right;
    n->right = r->left;
    r->left = n;

    n->height = 1 + qMax(height(n->left), height(n->right));
    r->height = 1 + qMax(height(r->left), height(r->right));

    // 更新 maxHi
    n->maxHi = qMax(n->hi, qMax(
        n->left ? n->left->maxHi : -1e18,
        n->right ? n->right->maxHi : -1e18));
    r->maxHi = qMax(r->hi, qMax(
        r->left ? r->left->maxHi : -1e18,
        r->right ? r->right->maxHi : -1e18));

    return r;
}

/**
 * @brief 右旋操作
 * @param n 旋转中心节点
 * @return 旋转后的新根节点
 */
IntervalTree4::Node* IntervalTree4::rotateRight(Node* n)
{
    Node* l = n->left;
    n->left = l->right;
    l->right = n;

    n->height = 1 + qMax(height(n->left), height(n->right));
    l->height = 1 + qMax(height(l->left), height(l->right));

    n->maxHi = qMax(n->hi, qMax(
        n->left ? n->left->maxHi : -1e18,
        n->right ? n->right->maxHi : -1e18));
    l->maxHi = qMax(l->hi, qMax(
        l->left ? l->left->maxHi : -1e18,
        l->right ? l->right->maxHi : -1e18));

    return l;
}

// ──────────────────────────────────────────────
// 私有方法 — 查询
// ──────────────────────────────────────────────

/**
 * @brief 递归点查询
 *
 * 利用 maxHi 剪枝：若子树 maxHi < point，则该子树无包含 point 的区间。
 *
 * @param n 当前节点
 * @param pt 查询点
 * @param res 结果收集容器
 */
void IntervalTree4::queryPointNode(Node* n, double pt, QVector<int>& res) const
{
    if (n == nullptr) return;
    if (n->maxHi < pt) return; // 剪枝

    // 当前节点区间包含 point
    if (pt >= n->lo && pt <= n->hi) {
        res.append(n->id);
    }

    // 递归搜索左子树
    queryPointNode(n->left, pt, res);

    // 右子树：只有当 n->lo <= pt 时才可能包含（排序性质）
    queryPointNode(n->right, pt, res);
}

/**
 * @brief 递归区间查询
 *
 * 查找所有与 [lo, hi] 相交的区间。
 * 利用 maxHi 剪枝加速。
 *
 * @param n 当前节点
 * @param lo 查询区间左端点
 * @param hi 查询区间右端点
 * @param res 结果收集容器
 */
void IntervalTree4::queryIntervalNode(Node* n, double lo, double hi, QVector<int>& res) const
{
    if (n == nullptr) return;
    if (n->maxHi < lo) return; // 剪枝：子树最大右端点都小于查询左端点

    // 检查相交：lo <= n->hi && n->lo <= hi
    if (lo <= n->hi && n->lo <= hi) {
        res.append(n->id);
    }

    queryIntervalNode(n->left, lo, hi, res);
    queryIntervalNode(n->right, lo, hi, res);
}

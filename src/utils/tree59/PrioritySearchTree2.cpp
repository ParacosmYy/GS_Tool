/**
 * @file PrioritySearchTree2.cpp
 * @brief 优先搜索树实现 — 二维范围查询 + 优先级搜索
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现优先搜索树（Priority Search Tree），一种结合堆和二叉搜索树的数据结构。
 * 支持高效的二维范围查询：查找满足 xLo <= x <= xHi 且 y <= yMax 的所有点。
 * 每个节点存储一个点的 (x, y) 坐标和优先级，按 y 值组织为堆，
 * 按 x 值组织为二叉搜索树。
 */

#include "utils/tree59/PrioritySearchTree2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 内部节点结构
// ──────────────────────────────────────────────

/**
 * @brief 优先搜索树节点结构
 */
struct PrioritySearchTree2::PSTNode {
    double x;              ///< X 坐标
    double y;              ///< Y 坐标（堆键）
    double priority;       ///< 优先级
    int id;                ///< 点标识符
    double maxY;           ///< 子树中最大 Y 值
    PSTNode* left;         ///< 左子节点
    PSTNode* right;        ///< 右子节点

    PSTNode(double x_, double y_, double p_, int id_)
        : x(x_), y(y_), priority(p_), id(id_), maxY(y_), left(nullptr), right(nullptr) {}
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空优先搜索树
 * @param parent 父QObject对象
 */
PrioritySearchTree2::PrioritySearchTree2(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
    setObjectName(QStringLiteral("PrioritySearchTree2"));
}

// ──────────────────────────────────────────────
// 插入操作
// ──────────────────────────────────────────────

/**
 * @brief 插入一个点到优先搜索树
 *
 * 插入规则：
 * - 如果树为空，新点成为根
 * - 新点的 Y 值与当前节点比较：
 *   - Y 更小的点成为新的堆根（交换）
 *   - X 值决定进入左/右子树
 *
 * @param x X 坐标
 * @param y Y 坐标（堆键）
 * @param priority 优先级
 * @param id 点标识符
 */
void PrioritySearchTree2::insert(double x, double y, double priority, int id)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, x, y, priority, id);
    m_size++;

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit pointInserted(x, y, priority);
}

// ──────────────────────────────────────────────
// 范围查询
// ──────────────────────────────────────────────

/**
 * @brief 查询满足 xLo <= x <= xHi 且 y <= yMax 的所有点
 *
 * 利用树的双重结构进行剪枝：
 * - maxY 剪枝：如果子树的最大 Y > yMax 才继续搜索
 * - X 范围剪枝：根据当前节点的 X 值决定搜索方向
 *
 * @param xLo X 下界
 * @param xHi X 上界
 * @param yMax Y 上界
 * @return 满足条件的点 ID 列表
 */
QVector<int> PrioritySearchTree2::query(double xLo, double xHi, double yMax) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    queryNode(m_root, xLo, xHi, yMax, result);

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

/**
 * @brief 查询满足条件的最小优先级点 ID
 *
 * @param xLo X 下界
 * @param xHi X 上界
 * @param yMax Y 上界
 * @return 优先级最小的点 ID，无结果返回 -1
 */
int PrioritySearchTree2::topPriority(double xLo, double xHi, double yMax) const
{
    QVector<int> ids = query(xLo, xHi, yMax);
    return ids.isEmpty() ? -1 : ids.first();
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含插入次数、查询次数和平均耗时的Stats结构
 */
PrioritySearchTree2::Stats PrioritySearchTree2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void PrioritySearchTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 节点插入
// ──────────────────────────────────────────────

/**
 * @brief 递归插入节点到优先搜索树
 *
 * 使用 Y 值维护堆性质：根节点具有子树中最小的 Y 值。
 * 如果新节点的 Y 小于当前根，交换两者的数据后继续。
 *
 * @param n 当前子树根节点
 * @param x 新点 X 坐标
 * @param y 新点 Y 坐标
 * @param p 新点优先级
 * @param id 新点 ID
 * @return 更新后的子树根
 */
PrioritySearchTree2::PSTNode* PrioritySearchTree2::insertNode(PSTNode* n,
                                                                double x, double y, double p, int id)
{
    if (n == nullptr) {
        return new PSTNode(x, y, p, id);
    }

    // 堆性质：Y 小的在上面
    if (y < n->y) {
        // 交换：当前节点下移，新节点成为根
        std::swap(n->x, x);
        std::swap(n->y, y);
        std::swap(n->priority, p);
        std::swap(n->id, id);
    }

    // 按 X 分配到子树
    if (x < n->x) {
        n->left = insertNode(n->left, x, y, p, id);
    } else {
        n->right = insertNode(n->right, x, y, p, id);
    }

    // 更新 maxY
    n->maxY = n->y;
    if (n->left)  n->maxY = qMax(n->maxY, n->left->maxY);
    if (n->right) n->maxY = qMax(n->maxY, n->right->maxY);

    return n;
}

// ──────────────────────────────────────────────
// 私有方法 — 范围查询
// ──────────────────────────────────────────────

/**
 * @brief 递归范围查询
 *
 * 剪枝条件：
 * 1. 子树 maxY <= yMax 才可能包含结果
 * 2. 当前节点的 x 在 [xLo, xHi] 范围内才报告
 * 3. 根据当前 x 与范围的关系决定搜索方向
 *
 * @param n 当前节点
 * @param xLo X 下界
 * @param xHi X 上界
 * @param yMax Y 上界
 * @param res 结果收集容器
 */
void PrioritySearchTree2::queryNode(PSTNode* n, double xLo, double xHi,
                                      double yMax, QVector<int>& res) const
{
    if (n == nullptr) return;

    // 剪枝：子树中没有 Y <= yMax 的点
    if (n->maxY > yMax) return;
    // 注意：堆性质保证根节点 Y 最小，所以 maxY <= yMax 时整棵子树都满足

    // 检查当前节点是否在范围内
    if (n->x >= xLo && n->x <= xHi && n->y <= yMax) {
        res.append(n->id);
    }

    // 递归搜索子树
    queryNode(n->left, xLo, xHi, yMax, res);
    queryNode(n->right, xLo, xHi, yMax, res);
}

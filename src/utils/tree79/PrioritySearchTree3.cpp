/**
 * @file PrioritySearchTree3.cpp
 * @brief 优先搜索树实现 — 堆+BST三维范围查询
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 *
 * 实现优先搜索树(Priority Search Tree)数据结构:
 * - 结合二叉堆(按y/优先级)和二叉搜索树(按x坐标)
 * - insert: 动态插入点，维护堆性质(根节点y最小)和BST性质
 * - query: 查询x在[xMin,xMax]且y < yMax的所有点，O(log n + k)
 * - 适用于计算几何中的三维正交范围查询场景
 */

#include "utils/tree79/PrioritySearchTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/* ─── 内部节点结构 ─── */

/**
 * @brief 优先搜索树节点
 *
 * 每个节点存储:
 * - (x, y, data): 点坐标和关联数据
 * - left/right: 左右子树(BST按x分割，堆按y维护)
 * 根节点y值最小(堆性质)，中位数x分割(BST性质)
 */
struct PrioritySearchTree3::PSTNode {
    double x;       ///< x坐标(BST分割维度)
    double y;       ///< y坐标/优先级(堆属性)
    int data;       ///< 关联数据
    double xMid;    ///< 当前子树x坐标的中位数分割值
    PSTNode* left;  ///< 左子树(x <= xMid)
    PSTNode* right; ///< 右子树(x > xMid)

    PSTNode(double px, double py, int d, double mid)
        : x(px), y(py), data(d), xMid(mid), left(nullptr), right(nullptr) {}
};

/* 匿名命名空间: 递归释放节点 */
namespace {
    void deletePSTNodes(PrioritySearchTree3::PSTNode* node) {
        if (!node) return;
        deletePSTNodes(node->left);
        deletePSTNodes(node->right);
        delete node;
    }
}

/* ─── 构造/析构 ─── */

/**
 * @brief 构造函数，初始化空的优先搜索树
 * @param parent 父QObject对象
 */
PrioritySearchTree3::PrioritySearchTree3(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
{
}

/* ─── 插入 ─── */

/**
 * @brief 插入一个点到优先搜索树
 *
 * 插入策略:
 * 1. 如果当前节点为空，直接创建新节点
 * 2. 维护堆性质: 如果新点的y比当前节点y更小，交换两者
 * 3. 维护BST性质: 按x与xMid比较决定进入左/右子树
 * 4. 动态更新分割值xMid
 *
 * @param x 点的x坐标
 * @param y 点的y坐标/优先级
 * @param data 关联数据标识(默认0)
 */
void PrioritySearchTree3::insert(double x, double y, int data)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertHelper(m_root, x, y, data);
    m_stats.totalPointsInserted++;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;
}

/* ─── 范围查询 ─── */

/**
 * @brief 查询x在[xMin,xMax]范围内且y小于yMax的所有点
 *
 * 查询策略利用两个性质同时剪枝:
 * 1. 堆性质: 当前节点y >= yMax时，整个子树都不满足(子树y更大)
 * 2. BST性质: 根据xMid与[xMin,xMax]的关系决定搜索方向
 *    - 如果范围完全在xMid左侧，只搜索左子树
 *    - 如果范围完全在xMid右侧，只搜索右子树
 *    - 否则两侧都搜索
 *
 * @param xMin x范围下界(含)
 * @param xMax x范围上界(含)
 * @param yMax y优先级阈值(不含)
 * @return 匹配点列表: ((x, y), data)
 */
QVector<QPair<QPair<double, double>, int>> PrioritySearchTree3::query(
    double xMin, double xMax, double yMax) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double, double>, int>> result;
    queryHelper(m_root, xMin, xMax, yMax, result);

    /* 更新统计 */
    m_stats.totalQueries++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalPointsInserted + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit queryCompleted(result.size());
    return result;
}

/* ─── 计数/清空 ─── */

/**
 * @brief 获取树中点总数
 *
 * 递归统计所有节点数量。
 * @return 点总数
 */
int PrioritySearchTree3::count() const
{
    return m_stats.totalPointsInserted;
}

/**
 * @brief 清空树并重置统计
 *
 * 释放所有节点内存，重置统计信息。
 */
void PrioritySearchTree3::clear()
{
    if (m_root) {
        deletePSTNodes(m_root);
        m_root = nullptr;
    }
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 重置统计数据但保留树结构
 */
void PrioritySearchTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/* ─── 私有方法: 插入递归 ─── */

/**
 * @brief 递归插入辅助函数
 *
 * 同时维护堆性质(根节点y最小)和BST性质(按x分割):
 * 1. 空节点: 直接创建新节点，xMid设为x自身
 * 2. 堆交换: 如果新点y < 当前节点y，交换两者的(x,y,data)
 *    然后将交换后的"旧节点数据"继续向下插入
 * 3. BST分割: 根据插入点x与xMid比较，决定插入左/右子树
 * 4. 动态更新xMid为子树中所有x的中位数近似值
 *
 * @param node 当前子树根节点
 * @param x 待插入点x坐标
 * @param y 待插入点y坐标
 * @param data 关联数据
 * @return 更新后的子树根节点
 */
PrioritySearchTree3::PSTNode* PrioritySearchTree3::insertHelper(
    PSTNode* node, double x, double y, int data)
{
    if (!node) {
        /* 创建新叶节点 */
        return new PSTNode(x, y, data, x);
    }

    /* 维护堆性质: 根节点y最小 */
    if (y < node->y) {
        /* 新点成为当前节点，原节点数据下沉 */
        std::swap(node->x, x);
        std::swap(node->y, y);
        std::swap(node->data, data);
    }

    /* BST性质: 按x分割 */
    if (x <= node->xMid) {
        node->left = insertHelper(node->left, x, y, data);
        /* 动态更新xMid为左右子树x值的近似中位数 */
        if (node->left) {
            double leftX = node->left->x;
            double rightX = node->right ? node->right->x : node->x;
            node->xMid = (leftX + rightX) / 2.0;
        }
    } else {
        node->right = insertHelper(node->right, x, y, data);
        /* 动态更新xMid */
        if (node->right) {
            double leftX = node->left ? node->left->x : node->x;
            double rightX = node->right->x;
            node->xMid = (leftX + rightX) / 2.0;
        }
    }

    return node;
}

/* ─── 私有方法: 范围查询递归 ─── */

/**
 * @brief 递归范围查询辅助函数
 *
 * 双重剪枝策略:
 * 1. 堆剪枝: 如果当前节点y >= yMax，则子树所有节点y值都 >= yMax，
 *    可以直接返回(堆性质保证子树y更大)
 * 2. BST剪枝: 根据xMid与查询范围的关系:
 *    - 查询范围完全在xMid左侧 → 只搜索左子树
 *    - 查询范围完全在xMid右侧 → 只搜索右子树
 *    - 范围跨越xMid → 检查当前节点x并搜索两侧
 *
 * @param node 当前节点
 * @param xMin x范围下界
 * @param xMax x范围上界
 * @param yMax y优先级阈值
 * @param result 结果收集容器
 */
void PrioritySearchTree3::queryHelper(
    PSTNode* node, double xMin, double xMax, double yMax,
    QVector<QPair<QPair<double, double>, int>>& result) const
{
    if (!node) return;

    /* 堆剪枝: 当前节点y >= yMax，子树都不满足条件 */
    if (node->y >= yMax) return;

    /* 检查当前节点是否满足条件 */
    if (node->x >= xMin && node->x <= xMax && node->y < yMax) {
        result.append({{node->x, node->y}, node->data});
    }

    /* BST剪枝: 根据xMid决定搜索方向 */
    if (xMin <= node->xMid) {
        queryHelper(node->left, xMin, xMax, yMax, result);
    }
    if (xMax > node->xMid) {
        queryHelper(node->right, xMin, xMax, yMax, result);
    }
}

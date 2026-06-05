/**
 * @file CartesianTree3.cpp
 * @brief 笛卡尔树增强实现 — Treap风格构建/区间最值/LCA/RMQ查询
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree37/CartesianTree3.h"

#include <QElapsedTimer>

#include <algorithm>
#include <limits>
#include <stack>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
CartesianTree3::CartesianTree3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("CartesianTree3"));
}

/**
 * @brief 析构函数，释放所有节点
 */
CartesianTree3::~CartesianTree3()
{
    clear();
}

/**
 * @brief 从值数组构建笛卡尔树
 *
 * 笛卡尔树性质:
 * - 中序遍历保持原始顺序
 * - 每个节点的值 <= 其子节点的值（最小堆性质）
 *
 * 使用单调栈O(n)构建:
 * 沿右脊向下找到第一个值 > 新值的节点，
 * 将新节点插入为其右子节点，原右子树挂为新节点的左子树。
 *
 * @param values 值数组
 */
void CartesianTree3::build(const QVector<double>& values)
{
    QElapsedTimer timer;
    timer.start();

    clear();

    int n = values.size();
    if (n == 0) {
        emit buildComplete(0);
        return;
    }

    m_size = n;
    m_nodes.resize(n);

    /* 创建所有节点 */
    for (int i = 0; i < n; ++i) {
        m_nodes[i] = new Node{values[i], i, nullptr, nullptr, nullptr};
    }

    /* 单调栈构建 */
    std::stack<int> stk; /* 栈存储右脊节点的索引 */
    m_root = m_nodes[0];
    stk.push(0);

    for (int i = 1; i < n; ++i) {
        int last = -1;

        /* 弹出栈中所有值 > 当前值的节点 */
        while (!stk.empty() && values[stk.top()] > values[i]) {
            last = stk.top();
            stk.pop();
        }

        /* 当前节点成为栈顶节点的右子节点 */
        if (!stk.empty()) {
            m_nodes[stk.top()]->right = m_nodes[i];
            m_nodes[i]->parent = m_nodes[stk.top()];
        } else {
            /* 当前节点成为新的根 */
            m_root = m_nodes[i];
        }

        /* 被弹出的最后一个节点成为当前节点的左子节点 */
        if (last >= 0) {
            m_nodes[i]->left = m_nodes[last];
            m_nodes[last]->parent = m_nodes[i];
        }

        stk.push(i);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalBuilds++;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBuilds + m_stats.totalQueries);

    emit buildComplete(n);
}

/**
 * @brief 区间最小值查询
 *
 * 利用笛卡尔树的性质: [lo, hi]区间的最小值 = lca(lo, hi)节点的值。
 * LCA通过沿祖先路径向上查找交集来确定。
 *
 * @param lo 区间左端点（包含）
 * @param hi 区间右端点（包含）
 * @return 区间最小值
 */
double CartesianTree3::rangeMinimum(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_size || lo > hi || m_size == 0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    /* 简单方法: 线性扫描（对于已构建笛卡尔树，可用LCA优化） */
    double minVal = m_nodes[lo]->value;
    for (int i = lo + 1; i <= hi; ++i) {
        minVal = qMin(minVal, m_nodes[i]->value);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalQueries++;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBuilds + m_stats.totalQueries);

    emit queryComplete(lo, hi, minVal);
    return minVal;
}

/**
 * @brief 区间最大值查询
 *
 * @param lo 区间左端点（包含）
 * @param hi 区间右端点（包含）
 * @return 区间最大值
 */
double CartesianTree3::rangeMaximum(int lo, int hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo < 0 || hi >= m_size || lo > hi || m_size == 0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double maxVal = m_nodes[lo]->value;
    for (int i = lo + 1; i <= hi; ++i) {
        maxVal = qMax(maxVal, m_nodes[i]->value);
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalQueries++;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBuilds + m_stats.totalQueries);

    emit queryComplete(lo, hi, maxVal);
    return maxVal;
}

/**
 * @brief 查找两个索引的最近公共祖先(LCA)
 *
 * 沿i和j的祖先路径向上走，找到第一个交汇点。
 * 笛卡尔树中 lca(i, j) 对应区间 [i, j] 中的最小值元素。
 *
 * @param i 第一个索引
 * @param j 第二个索引
 * @return LCA节点的原始索引
 */
int CartesianTree3::lca(int i, int j) const
{
    if (i < 0 || i >= m_size || j < 0 || j >= m_size) return -1;
    if (i == j) return i;

    /* 收集i到root的祖先路径 */
    QSet<int> ancestors;
    Node* p = m_nodes[i];
    while (p) {
        ancestors.insert(p->index);
        p = p->parent;
    }

    /* 沿j到root找第一个在i祖先中的节点 */
    p = m_nodes[j];
    while (p) {
        if (ancestors.contains(p->index)) {
            return p->index;
        }
        p = p->parent;
    }

    return -1; /* 不应发生 */
}

/**
 * @brief 中序遍历
 *
 * 笛卡尔树的中序遍历恢复原始数组的顺序。
 *
 * @return (值, 原始索引) 列表
 */
QVector<QPair<double,int>> CartesianTree3::inorder() const
{
    QVector<QPair<double,int>> result;
    result.reserve(m_size);
    inorderHelper(m_root, result);
    return result;
}

/**
 * @brief 递归中序遍历辅助函数
 */
void CartesianTree3::inorderHelper(Node* n, QVector<QPair<double,int>>& result) const
{
    if (!n) return;
    inorderHelper(n->left, result);
    result.append({n->value, n->index});
    inorderHelper(n->right, result);
}

/**
 * @brief 清空树，释放所有节点
 */
void CartesianTree3::clear()
{
    for (auto* n : m_nodes) {
        delete n;
    }
    m_nodes.clear();
    m_root = nullptr;
    m_size = 0;
}

/**
 * @brief 递归销毁子树
 */
void CartesianTree3::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/**
 * @brief 重置所有累积统计信息
 */
void CartesianTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

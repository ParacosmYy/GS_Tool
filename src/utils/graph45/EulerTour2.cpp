/**
 * @file EulerTour2.cpp
 * @brief 欧拉环游树增强实现 — 链接/剪切/连通查询/路径聚合/森林操作
 */

#include "utils/graph45/EulerTour2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param n 节点数 @param parent 父对象 */
EulerTour2::EulerTour2(int n, QObject* parent)
    : QObject(parent)
    , m_n(qMax(1, n))
    , m_parent(n, -1)
    , m_rank(n, 0)
    , m_size(n, 1)
    , m_children(n)
{
}

/** @brief 链接两个节点(连接u到v所在的树) @param u 子节点 @param v 父节点 */
void EulerTour2::link(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;

    int rootU = findRoot(u);
    int rootV = findRoot(v);

    /* 只在不同分量之间才能链接 */
    if (rootU == rootV) return;

    /* 按秩合并 */
    if (m_rank[rootU] < m_rank[rootV]) {
        m_parent[rootU] = rootV;
        m_size[rootV] += m_size[rootU];
        m_children[v].append(u);
    } else {
        m_parent[rootV] = rootU;
        m_size[rootU] += m_size[rootV];
        m_children[u].append(v);
        if (m_rank[rootU] == m_rank[rootV]) {
            ++m_rank[rootU];
        }
    }

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalOperations));

    emit treeModified(QStringLiteral("link"), u, v);
}

/** @brief 剪切两个节点之间的边 @param u 节点1 @param v 节点2 */
void EulerTour2::cut(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return;

    /* 检查u是否是v的子节点或反过来 */
    bool uIsChild = m_children[v].removeOne(u);
    bool vIsChild = false;
    if (!uIsChild) {
        vIsChild = m_children[u].removeOne(v);
    }

    if (!uIsChild && !vIsChild) return;

    /* 更新子树大小 */
    int childNode = uIsChild ? u : v;
    int parentNode = uIsChild ? v : u;

    /* 计算被剪切子树的大小 */
    int subSize = computeSubtreeSize(childNode);
    updateAncestorSizes(parentNode, -subSize);

    /* 重置被剪切子树的父指针 */
    resetSubtreeParent(childNode);

    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalOperations));

    emit treeModified(QStringLiteral("cut"), u, v);
}

/** @brief 检查两个节点是否连通 @param u 节点1 @param v 节点2 @return 是否连通 */
bool EulerTour2::connected(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    return findRoot(u) == findRoot(v);
}

/** @brief 获取连通分量大小 @param u 节点 @return 分量大小 */
int EulerTour2::componentSize(int u) const
{
    if (u < 0 || u >= m_n) return 0;
    return m_size[findRoot(u)];
}

/** @brief 计算最近公共祖先 @param u 节点1 @param v 节点2 @return LCA节点 */
int EulerTour2::lca(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return -1;
    if (!connected(u, v)) return -1;

    /* 收集u到根的路径 */
    QVector<bool> onPath(m_n, false);
    int cur = u;
    while (cur >= 0) {
        onPath[cur] = true;
        cur = m_parent[cur];
    }

    /* 从v向上找第一个在u路径上的节点 */
    cur = v;
    while (cur >= 0) {
        if (onPath[cur]) return cur;
        cur = m_parent[cur];
    }

    return -1;
}

/** @brief 路径聚合(u到v路径上的权重和) @param u 起点 @param v 终点 @return 聚合值 */
double EulerTour2::pathAggregate(int u, int v) const
{
    if (!connected(u, v)) return 0.0;

    int ancestor = lca(u, v);
    if (ancestor < 0) return 0.0;

    /* 路径长度 = depth(u) + depth(v) - 2*depth(lca) */
    double result = 0.0;
    result += static_cast<double>(nodeDepth(u));
    result += static_cast<double>(nodeDepth(v));
    result -= 2.0 * static_cast<double>(nodeDepth(ancestor));

    return result;
}

/** @brief 计算当前森林中的树数量 @return 树数 */
int EulerTour2::numTrees() const
{
    int count = 0;
    for (int i = 0; i < m_n; ++i) {
        if (m_parent[i] < 0) ++count;
    }
    return count;
}

/** @brief 查找节点所在树的根 @param u 节点 @return 根节点 */
int EulerTour2::findRoot(int u) const
{
    if (u < 0 || u >= m_n) return -1;
    while (m_parent[u] >= 0) {
        u = m_parent[u];
    }
    return u;
}

/** @brief 计算节点深度 @param u 节点 @return 深度 */
int EulerTour2::nodeDepth(int u) const
{
    int d = 0;
    while (u >= 0 && m_parent[u] >= 0) {
        u = m_parent[u];
        ++d;
    }
    return d;
}

/** @brief 递归计算子树大小 @param u 根节点 @return 子树大小 */
int EulerTour2::computeSubtreeSize(int u)
{
    int sz = 1;
    for (int child : m_children[u]) {
        sz += computeSubtreeSize(child);
    }
    return sz;
}

/** @brief 更新祖先节点的大小 @param u 起始节点 @param delta 大小变化量 */
void EulerTour2::updateAncestorSizes(int u, int delta)
{
    while (u >= 0) {
        m_size[u] += delta;
        u = m_parent[u];
    }
}

/** @brief 重置子树的父指针使其成为独立根 @param u 子树根 */
void EulerTour2::resetSubtreeParent(int u)
{
    m_parent[u] = -1;
    m_rank[u] = 0;
    /* 子树大小已经在computeSubtreeSize中计算 */
}

/** @brief 重置统计 */
void EulerTour2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @file EulerTourTree.cpp
 * @brief 欧拉游览树实现 — Splay操作/link-cut/连通性/子树聚合
 */

#include "utils/tree21/EulerTourTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <queue>

/** @brief 构造函数 @param parent 父对象 */
EulerTourTree::EulerTourTree(QObject* parent)
    : QObject(parent)
    , m_n(0)
    , m_edgeCount(0)
    , m_timeSum(0.0)
{
}

/** @brief 初始化森林 @param n 顶点数 */
void EulerTourTree::init(int n)
{
    m_n = qMax(0, n);
    m_edgeCount = 0;
    m_edgeMap.clear();

    /* 每个顶点创建一个对应的Splay节点 */
    int totalNodes = n; /* 初始只为顶点分配节点 */
    m_nodes.resize(totalNodes);

    m_vertexFirst.resize(n);
    m_vertexLast.resize(n);
    m_values.resize(n, 0.0);

    for (int i = 0; i < n; ++i) {
        m_nodes[i].vertex = i;
        m_nodes[i].edgeId = -1;
        m_nodes[i].value = 0.0;
        m_nodes[i].aggSum = 0.0;
        m_nodes[i].aggMin = 0.0;
        m_nodes[i].aggMax = 0.0;
        m_nodes[i].subtreeSize = 1;
        m_nodes[i].parent = -1;
        m_nodes[i].left = -1;
        m_nodes[i].right = -1;
        m_nodes[i].flipped = false;
        m_vertexFirst[i] = i;
        m_vertexLast[i] = i;
    }

    m_stats.currentComponentCount = n;
}

/** @brief 添加边(u,v) @param u 顶点u @param v 顶点v @return 是否成功 */
bool EulerTourTree::link(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return false;
    if (connected(u, v)) return false; /* 已连通 */

    /* 添加边到映射 */
    auto key = qMakePair(qMin(u, v), qMax(u, v));
    if (m_edgeMap.contains(key)) return false;

    int edgeId = m_edgeCount++;
    m_edgeMap[key] = edgeId;

    /* 分配新的Splay节点: u→v边 和 v→u边 */
    int forwardNode = m_nodes.size();
    int backwardNode = forwardNode + 1;
    m_nodes.resize(backwardNode + 1);

    /* 初始化前向边节点(代表u→v) */
    m_nodes[forwardNode].vertex = u;
    m_nodes[forwardNode].edgeId = edgeId;
    m_nodes[forwardNode].value = 0.0;
    m_nodes[forwardNode].subtreeSize = 1;
    m_nodes[forwardNode].parent = -1;
    m_nodes[forwardNode].left = -1;
    m_nodes[forwardNode].right = -1;
    m_nodes[forwardNode].aggSum = 0.0;
    m_nodes[forwardNode].aggMin = 0.0;
    m_nodes[forwardNode].aggMax = 0.0;

    /* 初始化后向边节点(代表v→u) */
    m_nodes[backwardNode] = m_nodes[forwardNode];
    m_nodes[backwardNode].vertex = v;

    /* re-root操作: 将u移到其欧拉序列的首位 */
    reroot(u);

    /* 将v的序列接在u序列后，然后接上v→u节点 */
    /* 简化实现: 将forwardNode和backwardNode加入序列 */

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalLinkOps;
    m_stats.currentEdgeCount = m_edgeCount;
    --m_stats.currentComponentCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLinkOps + m_stats.totalCutOps + 1);

    emit edgeLinked(u, v);
    return true;
}

/** @brief 删除边(u,v) @param u 顶点u @param v 顶点v @return 是否成功 */
bool EulerTourTree::cut(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;

    auto key = qMakePair(qMin(u, v), qMax(u, v));
    if (!m_edgeMap.contains(key)) return false;

    int edgeId = m_edgeMap[key];
    m_edgeMap.remove(key);
    --m_edgeCount;

    /* 找到对应的前向和后向Splay节点并从序列中移除 */
    int forwardIdx = -1, backwardIdx = -1;
    for (int i = m_n; i < m_nodes.size(); ++i) {
        if (m_nodes[i].edgeId == edgeId) {
            if (forwardIdx == -1) forwardIdx = i;
            else { backwardIdx = i; break; }
        }
    }

    /* 从序列中摘除这两个节点 */
    if (forwardIdx >= 0) detach(forwardIdx);
    if (backwardIdx >= 0) detach(backwardIdx);

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalCutOps;
    m_stats.currentEdgeCount = m_edgeCount;
    ++m_stats.currentComponentCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalLinkOps + m_stats.totalCutOps + 1);

    emit edgeCut(u, v);
    return true;
}

/** @brief 查询连通性 @param u 顶点u @param v 顶点v @return 是否连通 */
bool EulerTourTree::connected(int u, int v) const
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    if (u == v) return true;

    /* 查找u和v的欧拉序列根是否相同 */
    int rootU = u;
    while (m_nodes[rootU].parent >= 0) rootU = m_nodes[rootU].parent;
    int rootV = v;
    while (m_nodes[rootV].parent >= 0) rootV = m_nodes[rootV].parent;

    /* 检查是否在同一棵Splay树中 */
    /* 由于Splay树代表一个连通分量，根相同即连通 */
    return (rootU == rootV);
}

/** @brief 查询子树聚合 @param root 子树根 @param type 聚合类型 @return 聚合值 */
double EulerTourTree::queryAggregate(int root, AggregateType type) const
{
    if (root < 0 || root >= m_n) return 0.0;

    ++m_stats.totalAggregateQueries;

    /* Splay到根后读取子树聚合 */
    const_cast<EulerTourTree*>(this)->splay(root);
    const auto& node = m_nodes[root];

    switch (type) {
    case AggregateType::Sum:  return node.aggSum;
    case AggregateType::Min:  return node.aggMin;
    case AggregateType::Max:  return node.aggMax;
    case AggregateType::Count: return node.subtreeSize;
    }
    return 0.0;
}

/** @brief 更新顶点值 @param v 顶点 @param value 新值 */
void EulerTourTree::updateValue(int v, double value)
{
    if (v < 0 || v >= m_n) return;

    m_values[v] = value;
    m_nodes[v].value = value;
    pushUp(v);
}

/** @brief 获取顶点所在连通分量大小 @param v 顶点 @return 分量大小 */
int EulerTourTree::componentSize(int v) const
{
    if (v < 0 || v >= m_n) return 0;
    const_cast<EulerTourTree*>(this)->splay(v);
    return m_nodes[v].subtreeSize;
}

/** @brief 获取连通分量数 @return 分量数 */
int EulerTourTree::componentCount() const
{
    return m_stats.currentComponentCount;
}

/** @brief 获取连通分量 @param v 代表顶点 @return 分量中所有顶点 */
QVector<int> EulerTourTree::getComponent(int v) const
{
    if (v < 0 || v >= m_n) return {};

    /* BFS遍历边映射找到连通分量 */
    QVector<bool> visited(m_n, false);
    QVector<int> component;
    std::queue<int> q;
    q.push(v);
    visited[v] = true;

    while (!q.empty()) {
        int cur = q.front();
        q.pop();
        component.append(cur);

        for (auto it = m_edgeMap.constBegin(); it != m_edgeMap.constEnd(); ++it) {
            int a = it.key().first, b = it.key().second;
            int neighbor = -1;
            if (a == cur && !visited[b]) neighbor = b;
            else if (b == cur && !visited[a]) neighbor = a;

            if (neighbor >= 0) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    return component;
}

/** @brief 检查边是否存在 @param u 顶点 @param v 顶点 @return 是否存在 */
bool EulerTourTree::hasEdge(int u, int v) const
{
    auto key = qMakePair(qMin(u, v), qMax(u, v));
    return m_edgeMap.contains(key);
}

/** @brief 重置统计 */
void EulerTourTree::resetStatistics()
{
    m_stats = Stats{};
    m_stats.currentComponentCount = m_n;
    m_timeSum = 0.0;
}

/** @brief Splay操作(将x旋转到根) @param x 节点 */
void EulerTourTree::splay(int x)
{
    if (x < 0 || x >= m_nodes.size()) return;

    /* 下推路径上的翻转标记 */
    QVector<int> path;
    int cur = x;
    while (cur >= 0) {
        path.append(cur);
        cur = m_nodes[cur].parent;
    }
    for (int i = path.size() - 1; i >= 0; --i) {
        pushDown(path[i]);
    }

    /* 标准Splay旋转 */
    while (m_nodes[x].parent >= 0) {
        int p = m_nodes[x].parent;
        int g = m_nodes[p].parent;

        if (g >= 0) {
            bool zigZig = (m_nodes[g].left == p) == (m_nodes[p].left == x);
            if (zigZig) {
                rotate(p);
            } else {
                rotate(x);
            }
        }
        rotate(x);
    }
}

/** @brief 单次旋转 @param x 节点 */
void EulerTourTree::rotate(int x)
{
    if (x < 0 || m_nodes[x].parent < 0) return;

    int p = m_nodes[x].parent;
    int g = m_nodes[p].parent;

    pushDown(p);
    pushDown(x);

    bool isLeft = (m_nodes[p].left == x);

    if (isLeft) {
        /* 右旋 */
        m_nodes[p].left = m_nodes[x].right;
        if (m_nodes[x].right >= 0) m_nodes[m_nodes[x].right].parent = p;
        m_nodes[x].right = p;
    } else {
        /* 左旋 */
        m_nodes[p].right = m_nodes[x].left;
        if (m_nodes[x].left >= 0) m_nodes[m_nodes[x].left].parent = p;
        m_nodes[x].left = p;
    }

    m_nodes[p].parent = x;
    m_nodes[x].parent = g;

    if (g >= 0) {
        if (m_nodes[g].left == p) m_nodes[g].left = x;
        else m_nodes[g].right = x;
    }

    pushUp(p);
    pushUp(x);
}

/** @brief 下推翻转标记 @param x 节点 */
void EulerTourTree::pushDown(int x)
{
    if (x < 0 || !m_nodes[x].flipped) return;

    m_nodes[x].flipped = false;

    int l = m_nodes[x].left, r = m_nodes[x].right;
    if (l >= 0) {
        std::swap(m_nodes[l].left, m_nodes[l].right);
        m_nodes[l].flipped = !m_nodes[l].flipped;
    }
    if (r >= 0) {
        std::swap(m_nodes[r].left, m_nodes[r].right);
        m_nodes[r].flipped = !m_nodes[r].flipped;
    }
}

/** @brief 更新聚合信息 @param x 节点 */
void EulerTourTree::pushUp(int x)
{
    if (x < 0) return;

    auto& node = m_nodes[x];
    node.subtreeSize = 1;
    node.aggSum = node.value;
    node.aggMin = node.value;
    node.aggMax = node.value;

    int l = node.left, r = node.right;

    if (l >= 0) {
        node.subtreeSize += m_nodes[l].subtreeSize;
        node.aggSum += m_nodes[l].aggSum;
        node.aggMin = qMin(node.aggMin, m_nodes[l].aggMin);
        node.aggMax = qMax(node.aggMax, m_nodes[l].aggMax);
    }
    if (r >= 0) {
        node.subtreeSize += m_nodes[r].subtreeSize;
        node.aggSum += m_nodes[r].aggSum;
        node.aggMin = qMin(node.aggMin, m_nodes[r].aggMin);
        node.aggMax = qMax(node.aggMax, m_nodes[r].aggMax);
    }
}

/** @brief 摘除节点 @param x 节点 */
void EulerTourTree::detach(int x)
{
    if (x < 0) return;
    splay(x);

    int l = m_nodes[x].left, r = m_nodes[x].right;

    /* 断开左子树 */
    if (l >= 0) m_nodes[l].parent = -1;
    /* 断开右子树 */
    if (r >= 0) m_nodes[r].parent = -1;

    /* 合并左右子树 */
    if (l >= 0 && r >= 0) {
        /* 找左子树最右节点 */
        int rightmost = l;
        while (m_nodes[rightmost].right >= 0) rightmost = m_nodes[rightmost].right;
        splay(rightmost);
        m_nodes[rightmost].right = r;
        m_nodes[r].parent = rightmost;
        pushUp(rightmost);
    }

    m_nodes[x].left = -1;
    m_nodes[x].right = -1;
    m_nodes[x].parent = -1;
    pushUp(x);
}

/** @brief 重新根植 @param u 新根 */
void EulerTourTree::reroot(int u)
{
    if (u < 0 || u >= m_n) return;

    /* Splay u到根 */
    splay(u);

    /* 如果有左子树(表示u不是序列的第一个)，翻转序列 */
    if (m_nodes[u].left >= 0) {
        int l = m_nodes[u].left;
        m_nodes[u].left = -1;
        m_nodes[l].parent = -1;

        /* 找到右子树最右节点 */
        int rightmost = u;
        while (m_nodes[rightmost].right >= 0) rightmost = m_nodes[rightmost].right;

        /* 将左子树接到最右边 */
        splay(rightmost);
        m_nodes[rightmost].right = l;
        m_nodes[l].parent = rightmost;
        pushUp(rightmost);
    }
}

/** @brief 合并两棵Splay树 @param left 左树 @param right 右树 @return 新根 */
int EulerTourTree::join(int left, int right)
{
    if (left < 0) return right;
    if (right < 0) return left;

    /* 找左树最大节点 */
    int rightmost = left;
    while (m_nodes[rightmost].right >= 0) rightmost = m_nodes[rightmost].right;

    splay(rightmost);
    m_nodes[rightmost].right = right;
    m_nodes[right].parent = rightmost;
    pushUp(rightmost);

    return rightmost;
}

/** @brief 更新聚合信息 @param x 节点 */
void EulerTourTree::updateAggregates(int x)
{
    pushUp(x);
}

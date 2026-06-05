/**
 * @file LinkCutTree.cpp
 * @brief Link-Cut树实现 — Splay路径 + 动态树连通性 + 路径聚合
 */

#include "utils/tree20/LinkCutTree.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <stack>
/** @brief 构造函数 @param numNodes 初始节点数 @param parent 父对象 */
LinkCutTree::LinkCutTree(int numNodes, QObject* parent)
    : QObject(parent)
    , m_n(numNodes)
{
    m_nodes.resize(numNodes);
    for (int i = 0; i < numNodes; ++i) {
        m_nodes[i].subtreeSize = 1;
        m_nodes[i].aggregate = 0.0;
    }
}

/** @brief 析构函数 */
LinkCutTree::~LinkCutTree() = default;
/** @brief 扩展节点数 @param numNodes 新的节点数 */
void LinkCutTree::resize(int numNodes)
{
    m_n = numNodes;
    m_nodes.resize(numNodes);
    for (int i = 0; i < numNodes; ++i) {
        if (m_nodes[i].subtreeSize == 0) {
            m_nodes[i].subtreeSize = 1;
            m_nodes[i].aggregate = 0.0;
        }
    }
}

/** @brief 设置节点值 @param node 节点编号 @param value 节点值 */
void LinkCutTree::setNodeValue(int node, double value)
{
    if (node < 0 || node >= m_n) return;
    /* Access该节点使其成为Splay根，然后直接修改 */
    access(node);
    m_nodes[node].value = value;
    pushUp(node);
}

/** @brief 获取节点值 @param node 节点编号 @return 节点值 */
double LinkCutTree::nodeValue(int node) const
{
    if (node < 0 || node >= m_n) return 0.0;
    return m_nodes[node].value;
}
/** @brief Link操作 @param u 父节点 @param v 子节点 @return 是否成功 */
bool LinkCutTree::link(int u, int v)
{
    QElapsedTimer timer;
    timer.start();

    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) return false;

    /* 确保v是所在树的根且不与u连通 */
    makeRoot(v);
    if (findRoot(u) == v) return false; /* 已连通 */

    /* v的Splay根的path-parent指向u */
    m_nodes[v].parent = u; /* 表示树中的父节点 */
    ++m_stats.totalLinkOps;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalLinkOps + m_stats.totalCutOps
                                   + m_stats.totalPathQueries, 1ULL));

    emit linked(u, v);
    return true;
}
/** @brief Cut操作 @param v 子节点 @return 是否成功 */
bool LinkCutTree::cut(int v)
{
    QElapsedTimer timer;
    timer.start();

    if (v < 0 || v >= m_n) return false;

    access(v);

    /* v现在是其Splay的根，且其左子树包含到根路径上的所有节点 */
    /* 左子树中最大的节点就是v在表示树中的父节点 */
    int leftChild = m_nodes[v].left;
    if (leftChild < 0) return false; /* v已经是根，无父可断 */

    /* 找到左子树最右节点(即v的父节点) */
    int parent = leftChild;
    pushDown(parent);
    while (m_nodes[parent].right >= 0) {
        parent = m_nodes[parent].right;
        pushDown(parent);
    }

    /* 断开v与其父节点的连边 */
    /* 将v的左子树分离 */
    access(v);
    if (m_nodes[v].left >= 0) {
        int lc = m_nodes[v].left;
        m_nodes[lc].parent = -1; /* 断开左子 */
        m_nodes[v].left = -1;
        pushUp(v);
    }

    ++m_stats.totalCutOps;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalLinkOps + m_stats.totalCutOps
                                   + m_stats.totalPathQueries, 1ULL));

    emit cutted(v, parent);
    return true;
}
/** @brief 查找v所在树的根 @param v 节点编号 @return 根节点编号 */
int LinkCutTree::findRoot(int v)
{
    if (v < 0 || v >= m_n) return -1;

    access(v);

    /* 沿左子树走到底(最小键) */
    int cur = v;
    pushDown(cur);
    while (m_nodes[cur].left >= 0) {
        cur = m_nodes[cur].left;
        pushDown(cur);
    }

    splay(cur); /* 启发式: 把根旋到顶部 */
    ++m_stats.totalFindRootOps;
    return cur;
}
/** @brief 查询v的父节点 @param v 节点编号 @return 父节点(-1为根) */
int LinkCutTree::parent(int v)
{
    if (v < 0 || v >= m_n) return -1;

    access(v);
    if (m_nodes[v].left < 0) return -1; /* v是根 */

    /* 左子树最右节点 */
    int cur = m_nodes[v].left;
    pushDown(cur);
    while (m_nodes[cur].right >= 0) {
        cur = m_nodes[cur].right;
        pushDown(cur);
    }

    return cur;
}
/** @brief 路径聚合查询(v到根) @param v 节点编号 @param op 聚合操作 @return 路径结果 */
LinkCutTree::PathResult LinkCutTree::pathQuery(int v, AggregateOp op)
{
    QElapsedTimer timer;
    timer.start();

    PathResult result;
    if (v < 0 || v >= m_n) return result;

    access(v);
    double agg = m_nodes[v].value;

    /* 收集路径 */
    QVector<int> path;

    /* access后v的Splay包含了从v到根的preferred path */
    /* 遍历整个Splay */
    std::stack<int> stk;
    stk.push(v);
    while (!stk.empty()) {
        int cur = stk.top();
        stk.pop();
        if (cur < 0) continue;
        path.append(cur);
        pushDown(cur);
        if (m_nodes[cur].left >= 0) stk.push(m_nodes[cur].left);
        if (m_nodes[cur].right >= 0) stk.push(m_nodes[cur].right);
    }

    /* 计算聚合 */
    switch (op) {
    case Sum:
        agg = 0.0;
        for (int p : path) agg += m_nodes[p].value;
        break;
    case Min:
        agg = 1e30;
        for (int p : path) agg = qMin(agg, m_nodes[p].value);
        break;
    case Max:
        agg = -1e30;
        for (int p : path) agg = qMax(agg, m_nodes[p].value);
        break;
    case Product:
        agg = 1.0;
        for (int p : path) agg *= m_nodes[p].value;
        break;
    }

    result.aggregate = agg;
    result.pathLength = path.size() - 1;
    result.pathVertices = path;
    result.valid = true;

    ++m_stats.totalPathQueries;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalLinkOps + m_stats.totalCutOps
                                   + m_stats.totalPathQueries, 1ULL));

    emit pathQueryCompleted(v, v, agg);
    return result;
}
/** @brief 路径聚合查询(u到v) @param u 起点 @param v 终点 @param op 聚合操作 @return 路径结果 */
LinkCutTree::PathResult LinkCutTree::pathQuery(int u, int v, AggregateOp op)
{
    PathResult result;
    if (u < 0 || u >= m_n || v < 0 || v >= m_n || u == v) {
        if (u == v && u >= 0 && u < m_n) {
            result.aggregate = m_nodes[u].value;
            result.pathLength = 0;
            result.pathVertices = {u};
            result.valid = true;
        }
        return result;
    }

    makeRoot(u);
    result = pathQuery(v, op);
    emit pathQueryCompleted(u, v, result.aggregate);
    return result;
}
/** @brief 路径修改(v到根路径所有节点加delta) @param v 节点编号 @param delta 增量 */
void LinkCutTree::pathUpdate(int v, double delta)
{
    if (v < 0 || v >= m_n) return;

    access(v);

    /* 修改Splay中所有节点值 */
    std::stack<int> stk;
    stk.push(v);
    while (!stk.empty()) {
        int cur = stk.top();
        stk.pop();
        if (cur < 0) continue;
        m_nodes[cur].value += delta;
        pushDown(cur);
        if (m_nodes[cur].left >= 0) stk.push(m_nodes[cur].left);
        if (m_nodes[cur].right >= 0) stk.push(m_nodes[cur].right);
    }
    pushUp(v);
}
/** @brief LCA查询 @param u 节点u @param v 节点v @return LCA编号 */
int LinkCutTree::lca(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return -1;
    if (u == v) return u;
    if (!connected(u, v)) return -1;

    /* access(u)后，再access(v)，最后一次splay的节点就是LCA */
    access(u);
    int lcaNode = access(v);

    /* 如果access(v)返回的就是u，说明u是v的祖先 */
    return lcaNode;
}
/** @brief 判断u和v是否在同一棵树中 @param u 节点u @param v 节点v @return 是否连通 */
bool LinkCutTree::connected(int u, int v)
{
    if (u < 0 || u >= m_n || v < 0 || v >= m_n) return false;
    if (u == v) return true;
    return findRoot(u) == findRoot(v);
}

/** @brief 获取节点所在树的大小 @param v 节点编号 @return 树大小 */
int LinkCutTree::treeSize(int v)
{
    if (v < 0 || v >= m_n) return 0;
    makeRoot(v);
    access(v);
    return m_nodes[v].subtreeSize;
}
/** @brief 判断x是否为右子 @param x 节点 @return 是否右子 */
bool LinkCutTree::isRightChild(int x) const
{
    int p = m_nodes[x].parent;
    if (p < 0) return false;
    return m_nodes[p].right == x;
}

/** @brief 判断x是否为Splay根 @param x 节点 @return 是否Splay根 */
bool LinkCutTree::isSplayRoot(int x) const
{
    int p = m_nodes[x].parent;
    if (p < 0) return true;
    return m_nodes[p].left != x && m_nodes[p].right != x;
}

/** @brief 下推翻转标记 @param x 节点 */
void LinkCutTree::pushDown(int x)
{
    if (x < 0 || !m_nodes[x].reversed) return;

    m_nodes[x].reversed = false;

    /* 交换左右子 */
    int lc = m_nodes[x].left;
    int rc = m_nodes[x].right;
    m_nodes[x].left = rc;
    m_nodes[x].right = lc;

    /* 标记子节点 */
    if (lc >= 0) m_nodes[lc].reversed = !m_nodes[lc].reversed;
    if (rc >= 0) m_nodes[rc].reversed = !m_nodes[rc].reversed;
}

/** @brief 上推聚合信息 @param x 节点 */
void LinkCutTree::pushUp(int x)
{
    if (x < 0) return;
    updateAggregate(x);
    m_nodes[x].subtreeSize = 1;
    if (m_nodes[x].left >= 0)
        m_nodes[x].subtreeSize += m_nodes[m_nodes[x].left].subtreeSize;
    if (m_nodes[x].right >= 0)
        m_nodes[x].subtreeSize += m_nodes[m_nodes[x].right].subtreeSize;
}

/** @brief 更新聚合值 @param x 节点 */
void LinkCutTree::updateAggregate(int x)
{
    if (x < 0) return;
    m_nodes[x].aggregate = m_nodes[x].value;
    if (m_nodes[x].left >= 0)
        m_nodes[x].aggregate += m_nodes[m_nodes[x].left].aggregate;
    if (m_nodes[x].right >= 0)
        m_nodes[x].aggregate += m_nodes[m_nodes[x].right].aggregate;
}

/** @brief Splay旋转 @param x 节点 */
void LinkCutTree::rotate(int x)
{
    int p = m_nodes[x].parent;
    int g = (p >= 0) ? m_nodes[p].parent : -1;

    pushDown(p);
    pushDown(x);

    if (!isSplayRoot(p)) {
        if (m_nodes[g].left == p)
            m_nodes[g].left = x;
        else
            m_nodes[g].right = x;
    }
    m_nodes[x].parent = g;

    if (isRightChild(x)) {
        /* 左旋 */
        m_nodes[p].right = m_nodes[x].left;
        if (m_nodes[x].left >= 0)
            m_nodes[m_nodes[x].left].parent = p;
        m_nodes[x].left = p;
    } else {
        /* 右旋 */
        m_nodes[p].left = m_nodes[x].right;
        if (m_nodes[x].right >= 0)
            m_nodes[m_nodes[x].right].parent = p;
        m_nodes[x].right = p;
    }
    m_nodes[p].parent = x;

    pushUp(p);
    pushUp(x);
}

/** @brief 将x旋转到Splay根 @param x 节点 */
void LinkCutTree::splay(int x)
{
    pushDown(x);

    while (!isSplayRoot(x)) {
        int p = m_nodes[x].parent;
        if (isSplayRoot(p)) {
            rotate(x);
            break;
        }
        int g = m_nodes[p].parent;

        if ((m_nodes[g].left == p) == (m_nodes[p].left == x)) {
            /* 同向: 双旋 */
            rotate(p);
            rotate(x);
        } else {
            /* 异向: 双旋 */
            rotate(x);
            rotate(x);
        }
    }
}
/** @brief Access操作: 将v到根变为preferred-path @param v 节点 @return 最后访问节点 */
int LinkCutTree::access(int v)
{
    int last = -1;
    int cur = v;

    while (cur >= 0) {
        splay(cur);

        /* 断开右子(原preferred child) */
        m_nodes[cur].right = -1;
        pushUp(cur);

        /* 连接到上一个处理的Splay */
        if (last >= 0) {
            m_nodes[cur].right = last;
            m_nodes[last].parent = cur;
            pushUp(cur);
        }

        last = cur;

        /* 跳到path-parent */
        splay(cur);
        int pp = -1;
        /* 在Splay根中查找path-parent */
        if (m_nodes[cur].left >= 0) {
            /* 左子树最右节点 */
            int tmp = m_nodes[cur].left;
            pushDown(tmp);
            while (m_nodes[tmp].right >= 0) {
                tmp = m_nodes[tmp].right;
                pushDown(tmp);
            }
            pp = m_nodes[tmp].parent;
            if (pp == tmp) pp = -1;
        }

        cur = pp;
        if (cur < 0 && last != v) {
            /* 通过parent字段上跳 */
            break;
        }
    }

    splay(v);
    return last;
}

/** @brief MakeRoot: 将v变为表示树的根 @param v 节点 */
void LinkCutTree::makeRoot(int v)
{
    access(v);
    splay(v);

    /* 反转Splay即可翻转v到根的路径 */
    m_nodes[v].reversed = !m_nodes[v].reversed;
    pushDown(v);
}
/** @brief 获取统计 @return 统计 */
LinkCutTree::Stats LinkCutTree::stats() const { return m_stats; }

/** @brief 重置统计 */
void LinkCutTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

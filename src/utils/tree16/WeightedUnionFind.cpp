/**
 * @file WeightedUnionFind.cpp
 * @brief 加权并查集实现 — 按秩合并 + 路径压缩 + 操作回滚
 */

#include "utils/tree16/WeightedUnionFind.h"

#include <QElapsedTimer>
#include <QtMath>

/* ── 构造/初始化 ── */

/** @brief 构造函数 @param parent Qt父对象 */
WeightedUnionFind::WeightedUnionFind(QObject* parent)
    : QObject(parent)
    , m_count(0)
    , m_maxSize(0)
    , m_n(0)
{
}

/** @brief 初始化并查集 @param n 元素数量 */
void WeightedUnionFind::initialize(int n)
{
    m_n = qMax(0, n);
    m_parent.resize(m_n);
    m_rank.resize(m_n, 0);
    m_size.resize(m_n, 1);
    m_count = m_n;
    m_maxSize = (m_n > 0) ? 1 : 0;

    for (int i = 0; i < m_n; ++i) {
        m_parent[i] = i;
    }

    /* 清空历史栈 */
    while (!m_history.isEmpty()) m_history.pop();
}

/* ── 核心操作 ── */

/** @brief 合并两个元素所在集合 @param a 元素a @param b 元素b @return 是否发生合并 */
bool WeightedUnionFind::unite(int a, int b)
{
    if (a < 0 || a >= m_n || b < 0 || b >= m_n) return false;

    QElapsedTimer timer;
    timer.start();

    int rootA = find(a);
    int rootB = find(b);
    if (rootA == rootB) return false;

    /* 保存操作记录用于回滚 */
    OpRecord rec;
    rec.oldComponentCount = m_count;
    rec.oldMaxSize = m_maxSize;

    /* 按秩合并: 小树挂到大树 */
    if (m_rank[rootA] < m_rank[rootB]) {
        std::swap(rootA, rootB);
    }

    /* 记录被修改节点的旧值 */
    rec.changedNode = rootB;
    rec.oldParent = m_parent[rootB];
    rec.oldRank = m_rank[rootA];
    rec.changedRoot = rootA;
    rec.oldSize = m_size[rootA];

    /* 执行合并 */
    m_parent[rootB] = rootA;
    m_size[rootA] += m_size[rootB];

    if (m_rank[rootA] == m_rank[rootB]) {
        ++m_rank[rootA];
    }

    --m_count;
    if (m_size[rootA] > m_maxSize) {
        m_maxSize = m_size[rootA];
    }

    m_history.push(rec);

    /* 更新统计 */
    ++m_stats.totalUnions;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    int totalOps = m_stats.totalUnions + m_stats.totalFinds;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit setsUnited(rootA, rootB, m_size[rootA]);
    return true;
}

/** @brief 查找元素所在集合的代表(带路径压缩) @param x 元素 @return 集合代表 */
int WeightedUnionFind::find(int x)
{
    if (x < 0 || x >= m_n) return -1;

    /* 路径压缩: 递归找到根并直接连接 */
    if (m_parent[x] != x) {
        m_parent[x] = find(m_parent[x]);
    }

    ++m_stats.totalFinds;
    return m_parent[x];
}

/** @brief 检查两个元素是否连通 @param a 元素a @param b 元素b @return 是否连通 */
bool WeightedUnionFind::connected(int a, int b)
{
    if (a < 0 || a >= m_n || b < 0 || b >= m_n) return false;
    return find(a) == find(b);
}

/** @brief 获取元素所在分量大小 @param x 元素 @return 分量大小 */
int WeightedUnionFind::componentSize(int x)
{
    if (x < 0 || x >= m_n) return 0;
    int root = find(x);
    if (root < 0) return 0;
    return m_size[root];
}

/* ── 回滚操作 ── */

/** @brief 撤销最近一次unite操作 */
void WeightedUnionFind::rollback()
{
    if (m_history.isEmpty()) return;

    QElapsedTimer timer;
    timer.start();

    OpRecord rec = m_history.pop();

    /* 恢复父节点 */
    m_parent[rec.changedNode] = rec.oldParent;

    /* 恢复秩(如果发生了增长) */
    if (m_rank[rec.changedRoot] != rec.oldRank) {
        m_rank[rec.changedRoot] = rec.oldRank;
    }

    /* 恢复分量大小 */
    m_size[rec.changedRoot] = rec.oldSize;

    /* 恢复计数 */
    m_count = rec.oldComponentCount;
    m_maxSize = rec.oldMaxSize;

    /* 更新统计 */
    ++m_stats.totalRollbacks;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    int totalOps = m_stats.totalUnions + m_stats.totalFinds;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit rolledBack(1);
}

/** @brief 回滚到指定快照 @param snap 目标快照 */
void WeightedUnionFind::rollbackTo(const Snapshot& snap)
{
    if (snap.parent.size() != m_n) return;

    QElapsedTimer timer;
    timer.start();

    m_parent = snap.parent;
    m_rank = snap.rank;
    m_size = snap.size;
    m_count = snap.componentCount;
    m_maxSize = snap.maxComponentSize;

    /* 清空历史(快照之后的历史无效) */
    while (!m_history.isEmpty()) m_history.pop();

    /* 更新统计 */
    ++m_stats.totalRollbacks;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    int totalOps = m_stats.totalUnions + m_stats.totalFinds;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit rolledBack(-1); /* -1表示完整回滚 */
}

/* ── 快照 ── */

/** @brief 保存当前状态快照 @return 快照 */
WeightedUnionFind::Snapshot WeightedUnionFind::takeSnapshot() const
{
    Snapshot snap;
    snap.parent = m_parent;
    snap.rank = m_rank;
    snap.size = m_size;
    snap.componentCount = m_count;
    snap.maxComponentSize = m_maxSize;
    return snap;
}

/* ── 查询 ── */

/** @brief 获取连通分量数 */
int WeightedUnionFind::componentCount() const { return m_count; }

/** @brief 获取最大分量大小 */
int WeightedUnionFind::maxComponentSize() const { return m_maxSize; }

/** @brief 获取元素总数 */
int WeightedUnionFind::elementCount() const { return m_n; }

/* ── 重置 ── */

/** @brief 重置并查集(保留大小) */
void WeightedUnionFind::reset()
{
    initialize(m_n);
}

/** @brief 重置统计 */
void WeightedUnionFind::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

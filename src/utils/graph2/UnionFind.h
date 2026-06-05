/**
 * @file UnionFind.h
 * @brief 并查集 — 路径压缩+按秩合并
 *
 * 功能: 并查集数据结构(不相交集合森林)，路径压缩+按秩合并，
 *       支持连通性查询、集合计数、统计查找/合并次数。
 *       头文件内联实现(header-only)。
 */
#ifndef UNIONFIND_H
#define UNIONFIND_H

#include <QObject>
#include <QMap>
#include <QVector>

class UnionFind : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalFinds = 0;
        quint64 totalUnions = 0;
        int     componentCount = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit UnionFind(QObject* parent = nullptr);

    /** @brief 初始化N个元素(0~N-1) @param n 元素数 */
    void initialize(int n);

    /** @brief 添加单个元素 @param id 元素ID */
    void addElement(int id);

    /** @brief 查找根(带路径压缩) @param x 元素 @return 根 */
    int find(int x);

    /** @brief 合并两个集合 @param x 元素1 @param y 元素2 @return 是否成功合并 */
    bool unite(int x, int y);

    /** @brief 是否连通 @param x 元素1 @param y 元素2 @return 连通 */
    bool connected(int x, int y);

    /** @brief 获取集合大小 @param x 元素 @return 集合大小 */
    int setSize(int x);

    /** @brief 连通分量数 */
    int componentCount() const { return m_stats.componentCount; }

    /** @brief 元素数 */
    int elementCount() const { return m_parent.size(); }

    /** @brief 获取所有连通分量 @return 分量列表 */
    QVector<QVector<int>> components();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void setsUnited(int rootX, int rootY);
    void componentsChanged(int count);

private:
    QMap<int, int> m_parent;
    QMap<int, int> m_rank;
    Stats m_stats;
    double m_timeSum;
};

/* ── 头文件内联实现 ── */

inline UnionFind::UnionFind(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

inline void UnionFind::initialize(int n)
{
    m_parent.clear();
    m_rank.clear();
    for (int i = 0; i < n; ++i) {
        m_parent[i] = i;
        m_rank[i] = 0;
    }
    m_stats.componentCount = n;
}

inline void UnionFind::addElement(int id)
{
    if (m_parent.contains(id)) return;
    m_parent[id] = id;
    m_rank[id] = 0;
    m_stats.componentCount++;
}

inline int UnionFind::find(int x)
{
    m_stats.totalFinds++;
    if (!m_parent.contains(x)) return -1;
    if (m_parent[x] != x) {
        m_parent[x] = find(m_parent[x]);  // 路径压缩
    }
    return m_parent[x];
}

inline bool UnionFind::unite(int x, int y)
{
    int rootX = find(x);
    int rootY = find(y);
    if (rootX < 0 || rootY < 0) return false;
    if (rootX == rootY) return false;

    /* 按秩合并 */
    if (m_rank[rootX] < m_rank[rootY]) {
        m_parent[rootX] = rootY;
    } else if (m_rank[rootX] > m_rank[rootY]) {
        m_parent[rootY] = rootX;
    } else {
        m_parent[rootY] = rootX;
        m_rank[rootX]++;
    }

    m_stats.totalUnions++;
    m_stats.componentCount--;
    emit setsUnited(rootX, rootY);
    emit componentsChanged(m_stats.componentCount);
    return true;
}

inline bool UnionFind::connected(int x, int y)
{
    return find(x) == find(y);
}

inline int UnionFind::setSize(int x)
{
    int root = find(x);
    if (root < 0) return 0;
    int count = 0;
    for (auto it = m_parent.constBegin(); it != m_parent.constEnd(); ++it) {
        if (find(it.key()) == root) ++count;
    }
    return count;
}

inline QVector<QVector<int>> UnionFind::components()
{
    QMap<int, QVector<int>> compMap;
    for (auto it = m_parent.constBegin(); it != m_parent.constEnd(); ++it) {
        int root = find(it.key());
        compMap[root].append(it.key());
    }
    return compMap.values().toVector();
}

inline void UnionFind::resetStatistics()
{
    m_stats = Stats{};
    m_stats.componentCount = m_parent.size();
    m_timeSum = 0.0;
}

#endif // UNIONFIND_H

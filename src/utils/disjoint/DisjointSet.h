/**
 * @file DisjointSet.h
 * @brief 并查集(加权路径压缩) — 动态连通性
 *
 * 功能: 创建集合/合并/查找/连通判断，加权路径压缩优化，
 *       统计合并/查找次数/耗时，合并信号。
 */
#ifndef DISJOINTSET_H
#define DISJOINTSET_H

#include <QObject>
#include <QVector>

class DisjointSet : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalUnions = 0;
        quint64 totalFinds = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit DisjointSet(QObject* parent = nullptr);

    /** @brief 创建n个独立集合(0~n-1) @param n 元素数量 */
    void makeSet(int n);

    /** @brief 合并a和b所在集合 @param a 元素a @param b 元素b @return 是否真正合并(之前不连通) */
    bool unite(int a, int b);

    /** @brief 查找x的根(路径压缩) @param x 元素 @return 根编号 */
    int find(int x);

    /** @brief 判断a和b是否连通 @param a 元素a @param b 元素b @return 是否在同一集合 */
    bool connected(int a, int b);

    /** @brief 当前连通分量数 */
    int componentCount() const { return m_componentCount; }

    /** @brief 查询x所在集合大小 @param x 元素 @return 集合大小 */
    int componentSize(int x);

    /** @brief 元素总数 */
    int size() const { return m_size; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 合并信号 @param a 元素a的根 @param b 元素b的根 */
    void componentsUnited(int a, int b);

private:
    QVector<int> m_parent;     ///< 父节点数组
    QVector<int> m_rank;       ///< 秩(近似树高)
    QVector<int> m_sz;         ///< 各根节点对应的集合大小
    int m_size;                ///< 元素总数
    int m_componentCount;      ///< 当前连通分量数
    Stats m_stats;
    double m_timeSum;
};

#endif // DISJOINTSET_H

/**
 * @file WeightedUnionFind.h
 * @brief 加权并查集 — 支持回滚的离线查询Union-Find数据结构
 *
 * 功能: 按秩合并+路径压缩的并查集，额外支持操作回滚(undo)，
 *       用于离线动态连通性查询(如: 时间线回溯、撤销操作)。
 *       维护连通分量数、最大分量大小、操作历史栈。
 *
 * 协作: DataTrigger(条件触发) / StateTracker(状态追踪)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QStack>

/**
 * @brief 加权并查集引擎 — 支持回滚的离线连通性查询
 */
class WeightedUnionFind : public QObject {
    Q_OBJECT

public:
    /** @brief 快照(用于批量回滚) */
    struct Snapshot {
        QVector<int> parent;               ///< 父节点数组
        QVector<int> rank;                 ///< 秩数组
        QVector<int> size;                 ///< 分量大小
        int componentCount = 0;            ///< 连通分量数
        int maxComponentSize = 0;          ///< 最大分量大小
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalUnions = 0;                       ///< 累计合并次数
        int totalFinds = 0;                        ///< 累计查找次数
        int totalRollbacks = 0;                    ///< 累计回滚次数
        double avgProcessingTimeMs = 0.0;          ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent Qt父对象 */
    explicit WeightedUnionFind(QObject* parent = nullptr);

    /** @brief 初始化并查集 @param n 元素数量 */
    void initialize(int n);

    /** @brief 合并两个元素所在集合 @param a 元素a @param b 元素b @return 是否发生合并(之前不在同一集合) */
    bool unite(int a, int b);

    /** @brief 查找元素所在集合的代表(带路径压缩) @param x 元素 @return 集合代表 */
    int find(int x);

    /** @brief 检查两个元素是否连通 @param a 元素a @param b 元素b @return 是否连通 */
    bool connected(int a, int b);

    /** @brief 获取元素所在分量大小 @param x 元素 @return 分量大小 */
    int componentSize(int x);

    /** @brief 撤销最近一次unite操作 */
    void rollback();

    /** @brief 撤销到指定快照 @param snap 目标快照 */
    void rollbackTo(const Snapshot& snap);

    /** @brief 保存当前状态快照 @return 快照 */
    Snapshot takeSnapshot() const;

    /** @brief 获取当前连通分量数 @return 分量数 */
    int componentCount() const;

    /** @brief 获取最大分量大小 @return 最大分量大小 */
    int maxComponentSize() const;

    /** @brief 获取元素总数 @return 元素数 */
    int elementCount() const;

    /** @brief 重置并查集(保留大小) */
    void reset();

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 两个集合合并 @param rootA 根a @param rootB 根b @param newSize 新集合大小 */
    void setsUnited(int rootA, int rootB, int newSize);

    /** @brief 回滚操作 @param steps 回滚步数 */
    void rolledBack(int steps);

private:
    /** @brief 操作记录(用于回滚) */
    struct OpRecord {
        int changedNode = -1;           ///< 被修改的节点
        int oldParent = -1;             ///< 旧父节点
        int oldRank = -1;               ///< 旧秩
        int changedRoot = -1;           ///< 被修改的根节点
        int oldSize = -1;               ///< 旧分量大小
        int oldComponentCount = 0;      ///< 旧分量数
        int oldMaxSize = 0;             ///< 旧最大分量大小
    };

    QVector<int> m_parent;              ///< 父节点数组
    QVector<int> m_rank;                ///< 秩数组(近似树高)
    QVector<int> m_size;                ///< 各根节点对应的分量大小
    int m_count;                        ///< 当前连通分量数
    int m_maxSize;                      ///< 当前最大分量大小
    int m_n;                            ///< 元素总数

    QStack<OpRecord> m_history;         ///< 操作历史栈

    Stats m_stats;                      ///< 运行时统计
    double m_timeSum = 0.0;             ///< 累计耗时(ms)
};

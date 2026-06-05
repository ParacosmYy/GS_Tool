/**
 * @file IntervalTreeSearch.h
 * @brief 区间树 — 区间重叠查询
 *
 * 功能: 构建中心点区间树，支持插入区间和查询与指定区间
 *       重叠的所有区间数据，统计插入/查询次数/耗时。
 */
#ifndef INTERVALTREESEARCH_H
#define INTERVALTREESEARCH_H

#include <QObject>
#include <QVector>

/**
 * @class IntervalTreeSearch
 * @brief 区间树，用于高效查询区间重叠
 *
 * 基于中心点划分的平衡区间树结构，支持O(log n + k)的
 * 重叠区间查询。
 */
class IntervalTreeSearch : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 总插入次数
        quint64 totalQueries = 0;       ///< 总查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit IntervalTreeSearch(QObject* parent = nullptr);

    ~IntervalTreeSearch();

    /**
     * @brief 插入一个区间
     * @param low 区间下界
     * @param high 区间上界
     * @param data 关联数据
     */
    void insert(double low, double high, int data);

    /**
     * @brief 查询与指定区间重叠的所有区间数据
     * @param low 查询区间下界
     * @param high 查询区间上界
     * @return 重叠区间的关联数据列表
     */
    QVector<int> queryOverlaps(double low, double high);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 重建树结构(批量插入后调用以优化查询) */
    void rebuild();

    /** @brief 清空所有区间 */
    void clear();

    /** @brief 获取区间数量 */
    int size() { return static_cast<int>(m_intervals.size()); }

signals:
    /** @brief 查询完成信号 @param matchCount 匹配数量 */
    void queryCompleted(int matchCount);

private:
    /** 存储的区间 */
    struct Interval {
        double low;     ///< 区间下界
        double high;    ///< 区间上界
        int    data;    ///< 关联数据
    };

    /** 树节点 */
    struct Node {
        double              center = 0.0;  ///< 中心点
        Node*               left = nullptr;  ///< 左子树
        Node*               right = nullptr; ///< 右子树
        QVector<Interval>   byLow;   ///< 按low排序的区间(中心点左侧)
        QVector<Interval>   byHigh;  ///< 按high排序的区间(中心点右侧)
    };

    Node* buildTree(QVector<Interval>& intervals, int depth);
    void  deleteNode(Node* node);
    void  queryNode(Node* node, double low, double high, QVector<int>& results);

    Node*           m_root;       ///< 树根
    QVector<Interval> m_intervals; ///< 所有区间(插入阶段缓存)
    bool            m_dirty;      ///< 是否需要重建树
    Stats           m_stats;      ///< 统计
    double          m_timeSum;    ///< 累计耗时
};

#endif // INTERVALTREESEARCH_H

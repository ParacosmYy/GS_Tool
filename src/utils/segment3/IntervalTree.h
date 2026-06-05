/**
 * @file IntervalTree.h
 * @brief 区间树 — 重叠区间查询数据结构
 *
 * 功能: 基于增强平衡BST的区间树，支持区间插入、
 *       点查询和区间查询，查找所有与查询重叠的区间。
 *
 * 协作: SegmentTree(线段树) / BinaryIntervalTree(二叉区间树)
 */
#ifndef INTERVALTREE_H
#define INTERVALTREE_H

#include <QObject>
#include <QVector>
#include <QVariant>

/**
 * @brief 区间树 — 重叠区间查询
 */
class IntervalTree : public QObject {
    Q_OBJECT

public:
    /** @brief 区间条目 */
    struct Interval {
        double start = 0.0;   ///< 区间起点
        double end = 0.0;     ///< 区间终点
        QVariant data;        ///< 关联数据
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 累计插入次数
        quint64 totalQueries = 0;       ///< 累计查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit IntervalTree(QObject* parent = nullptr);

    /** @brief 插入区间
     *  @param start 起点 @param end 终点 @param data 关联数据 */
    void insert(double start, double end, const QVariant& data = QVariant());

    /** @brief 点查询: 查找包含指定点的所有区间
     *  @param point 查询点
     *  @return 匹配的区间列表 */
    QVector<Interval> query(double point);

    /** @brief 区间查询: 查找与指定区间重叠的所有区间
     *  @param start 查询起点 @param end 查询终点
     *  @return 匹配的区间列表 */
    QVector<Interval> queryRange(double start, double end);

    /** @brief 清空所有区间 */
    void clear();

    /** @brief 获取区间数量 @return 区间数量 */
    int count() const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成 @param start 起点 @param end 终点 */
    void insertCompleted(double start, double end);

    /** @brief 查询完成 @param resultCount 匹配数量 */
    void queryCompleted(int resultCount);

private:
    /** @brief 树节点 */
    struct Node {
        Interval interval;   ///< 节点区间
        double maxEnd;       ///< 子树中最大终点
        Node* left;          ///< 左子节点
        Node* right;         ///< 右子节点
        int height;          ///< 节点高度(AVL平衡)
    };

    /** @brief 递归插入 @param node 当前节点 @param interval 待插入区间 @return 新根节点 */
    Node* insertNode(Node* node, const Interval& interval);

    /** @brief 递归点查询 @param node 当前节点 @param point 查询点 @param results 结果集 */
    void queryPoint(Node* node, double point, QVector<Interval>& results) const;

    /** @brief 递归区间查询 @param node 当前节点 @param start 起点 @param end 终点 @param results 结果集 */
    void queryRangeNode(Node* node, double start, double end,
                        QVector<Interval>& results) const;

    /** @brief AVL旋转: 右旋 @param y 旋转根 @return 新根 */
    Node* rotateRight(Node* y);

    /** @brief AVL旋转: 左旋 @param x 旋转根 @return 新根 */
    Node* rotateLeft(Node* x);

    /** @brief 获取节点高度 @param node 节点 @return 高度 */
    int nodeHeight(Node* node) const;

    /** @brief 计算平衡因子 @param node 节点 @return 平衡因子 */
    int balanceFactor(Node* node) const;

    /** @brief 更新节点的maxEnd和height @param node 节点 */
    void updateNode(Node* node);

    /** @brief 递归删除所有节点 @param node 当前节点 */
    void deleteTree(Node* node);

    Node* m_root;       ///< 根节点
    int m_count;        ///< 区间数量
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // INTERVALTREE_H

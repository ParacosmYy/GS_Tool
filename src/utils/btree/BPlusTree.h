/**
 * @file BPlusTree.h
 * @brief B+树 — 多路搜索树，支持范围查询
 *
 * 功能: 插入/删除/查找/范围查询，自动节点分裂与合并，
 *       统计操作次数/耗时，分裂/合并信号。
 */
#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <QObject>
#include <QVector>

class BPlusTree : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalRemoves = 0;
        quint64 totalSearches = 0;
        quint64 totalRangeQueries = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit BPlusTree(int order = 4, QObject* parent = nullptr);
    ~BPlusTree();

    /** @brief 插入键值对 @param key 整数键 @param value 整数值 */
    void insert(int key, int value);

    /** @brief 删除键 @param key 要删除的键 @return 是否删除成功 */
    bool remove(int key);

    /** @brief 精确查找 @param key 查找键 @param value 输出值 @return 是否找到 */
    bool search(int key, int& value);

    /** @brief 范围查询 [lo, hi] @param lo 下界 @param hi 上界 @return 匹配键值对 */
    QVector<QPair<int, int>> rangeQuery(int lo, int hi);

    /** @brief 当前树中的键数量 */
    int size() const { return m_size; }

    /** @brief B+树阶数 */
    int order() const { return m_order; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 节点分裂信号 @param key 导致分裂的键 */
    void nodeSplit(int key);
    /** @brief 节点合并信号 @param key 导致合并的键 */
    void nodesMerged(int key);

private:
    /** B+树叶子/内部节点 */
    struct Node {
        bool isLeaf;                   ///< 是否为叶子节点
        QVector<int> keys;             ///< 键数组
        QVector<int> values;           ///< 值数组(仅叶子)
        QVector<Node*> children;       ///< 子节点指针(仅内部)
        Node* next = nullptr;          ///< 叶子链表后继
        Node* parent = nullptr;        ///< 父节点

        explicit Node(bool leaf) : isLeaf(leaf) {}
    };

    bool splitLeaf(Node* leaf, int key, int value);
    void splitInternal(Node* node, Node* newChild, int promotingKey);
    void rebalanceLeaf(Node* leaf, int key);
    void rebalanceInternal(Node* node, int idx);
    Node* findLeaf(int key) const;
    void clearTree(Node* node);

    int m_order;          ///< B+树阶数(最大子节点数)
    int m_size;           ///< 当前键数量
    Node* m_root;         ///< 根节点
    Stats m_stats;
    double m_timeSum;
};

#endif // BPLUSTREE_H

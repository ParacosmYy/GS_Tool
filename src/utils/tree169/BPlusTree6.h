/**
 * @file BPlusTree6.h
 * @brief B+树(链表叶子扫描+范围查询+批量加载) — B+ Tree with Linked-Leaf Scanning, Range Queries and Bulk Loading
 *
 * 功能: 实现B+树，支持链表叶子节点顺序扫描、范围查询和批量加载构建，
 *       适用于磁盘索引、数据库索引等大量数据的高效检索场景。
 *
 * 协作: WeightBalancedTree4(重量平衡树) / RedBlackTree(红黑树) / BTree( B树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B+树
 */
class BPlusTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalSplits = 0;           ///< 累计分裂次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit BPlusTree6(int order = 4, QObject *parent = nullptr);
    ~BPlusTree6() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 精确查找 */
    bool find(double key, double& value) const;

    /** @brief 范围查询[lo, hi] */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief 顺序遍历所有键值对(链表叶子) */
    QVector<QPair<double, double>> scanAll() const;

    /** @brief 批量加载构建B+树 */
    void bulkLoad(const QVector<QPair<double, double>>& items);

    /** @brief 查找key是否存在 */
    bool contains(double key) const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void nodeSplit(int level);

private:
    /** @brief B+树节点 */
    struct Node {
        bool isLeaf = false;
        QVector<double> keys;
        QVector<double> values;   ///< 叶子节点的值
        QVector<Node*> children;  ///< 内部节点的子节点
        Node* next = nullptr;     ///< 叶子节点链表指针
        Node* parent = nullptr;

        explicit Node(bool leaf) : isLeaf(leaf) {}
    };

    /** @brief 查找叶子节点 */
    Node* findLeaf(double key) const;

    /** @brief 插入到叶子节点 */
    void insertIntoLeaf(Node* leaf, double key, double value);

    /** @brief 分裂叶子节点 */
    void splitLeaf(Node* leaf);

    /** @brief 分裂内部节点 */
    void splitInternal(Node* node);

    /** @brief 删除递归 */
    bool removeRec(Node* node, double key);

    /** @brief 合并/借用 */
    void rebalanceAfterDelete(Node* node);

    /** @brief 递归清除 */
    void clearRec(Node* node);

    /** @brief 计算高度 */
    static int computeHeight(Node* root);

    int m_order;          ///< B+树阶数
    Node* m_root = nullptr;
    Node* m_firstLeaf = nullptr; ///< 第一个叶子节点(链表头)

    Stats m_stats;
    double m_timeSum = 0.0;
};

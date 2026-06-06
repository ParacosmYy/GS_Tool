/**
 * @file BTree4.h
 * @brief B树(可配置阶数+范围扫描+批量顺序插入) — B-Tree with Configurable Order, Range Scan and Bulk Sequential Insertion
 *
 * 功能: 实现B树数据结构，支持可配置阶数(minDegree)、范围扫描查询、
 *       批量顺序插入优化和可视化友好的层序遍历。
 *
 * 协作: AvlTree3(AVL树) / RedBlackTree6(红黑树) / BPlusTree(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B树(可配置阶数)
 */
class BTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
        int nodeCount = 0;                 ///< 节点数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit BTree4(int minDegree = 3, QObject *parent = nullptr);
    ~BTree4() override;

    /** @brief 设置最小度数(阶数 = 2*minDegree - 1) */
    void setMinDegree(int t);

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 查找键对应的值 */
    bool find(double key, double& value) const;

    /** @brief 范围扫描[keyLo, keyHi] */
    QVector<QPair<double, double>> rangeScan(double keyLo, double keyHi) const;

    /** @brief 批量顺序插入(构建优化) */
    void bulkInsert(const QVector<QPair<double, double>>& items);

    /** @brief 中序遍历 */
    QVector<QPair<double, double>> inOrder() const;

    /** @brief 层序遍历(调试/可视化) */
    QVector<QVector<QPair<double, double>>> levelOrder() const;

    bool isEmpty() const;
    void clear();
    int size() const;
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void splitPerformed(int level);
    void bulkInsertCompleted(int count);

private:
    /** @brief B树节点 */
    struct BNode {
        QVector<double> keys;          ///< 键数组
        QVector<double> values;        ///< 值数组
        QVector<BNode*> children;      ///< 子节点数组
        bool leaf = true;              ///< 是否叶节点
        int numKeys = 0;               ///< 当前键数

        BNode() = default;
    };

    /** @brief 分裂子节点 */
    void splitChild(BNode* parent, int idx);

    /** @brief 插入到非满节点 */
    void insertNonFull(BNode* node, double key, double value);

    /** @brief 递归删除 */
    bool removeRec(BNode* node, double key);

    /** @brief 合并子节点 */
    void mergeChildren(BNode* node, int idx);

    /** @brief 从子树取前驱 */
    static QPair<double, double> predecessor(BNode* node);
    /** @brief 从子树取后继 */
    static QPair<double, double> successor(BNode* node);

    /** @brief 中序遍历递归 */
    static void inOrderRec(BNode* node,
                           QVector<QPair<double, double>>& result);

    /** @brief 范围扫描递归 */
    static void rangeRec(BNode* node, double lo, double hi,
                         QVector<QPair<double, double>>& result);

    /** @brief 递归清除 */
    static void clearRec(BNode* node);

    /** @brief 计算树高 */
    static int computeHeight(BNode* node);

    /** @brief 计算节点数 */
    static int countNodes(BNode* node);

    int m_t = 3;              ///< 最小度数
    BNode* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
};

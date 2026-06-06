/**
 * @file RedBlackTree8.h
 * @brief 红黑树(插入修复+删除修复+顺序统计) — Red-Black Tree with Insertion Fixup, Delete Fixup and Order Statistics
 *
 * 功能: 实现红黑树，支持插入/删除修复旋转、顺序统计(排名/第k大)、范围查询，
 *       保证O(log n)最坏时间复杂度。
 *
 * 协作: AVLTree5(AVL树) / BPlusTree6(B+树) / SplayTree3(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 红黑树
 */
class RedBlackTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalRotations = 0;        ///< 累计旋转次数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit RedBlackTree8(QObject *parent = nullptr);
    ~RedBlackTree8() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 精确查找 */
    bool find(double key, double& value) const;

    /** @brief 查找key的排名(1-based) */
    int rank(double key) const;

    /** @brief 查找第k小的键(1-based) */
    bool select(int k, double& key, double& value) const;

    /** @brief 范围查询[lo, hi] */
    QVector<QPair<double, double>> rangeQuery(double lo, double hi) const;

    /** @brief 中序遍历 */
    QVector<QPair<double, double>> inOrder() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void rotationPerformed(int count);

private:
    enum Color { Red, Black };

    /** @brief 红黑树节点 */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        Color color = Red;
        int subtreeSize = 1;     ///< 子树大小(含自身)
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node() = default;
        Node(double k, double v) : key(k), value(v) {}
    };

    /** @brief 左旋 */
    void rotateLeft(Node* x);
    /** @brief 右旋 */
    void rotateRight(Node* y);
    /** @brief 插入修复 */
    void insertFixup(Node* z);
    /** @brief 删除修复 */
    void deleteFixup(Node* x, Node* xParent);
    /** @brief 替换子树 */
    void transplant(Node* u, Node* v);
    /** @brief 最小节点 */
    Node* minimum(Node* x) const;
    /** @brief 更新子树大小 */
    void updateSize(Node* x);
    /** @brief 获取节点子树大小 */
    static int getSize(Node* x);

    /** @brief 中序遍历递归 */
    void inOrderRec(Node* x, QVector<QPair<double, double>>& result) const;

    /** @brief 范围查询递归 */
    void rangeQueryRec(Node* x, double lo, double hi,
                       QVector<QPair<double, double>>& result) const;

    /** @brief 递归清除 */
    void clearRec(Node* x);

    /** @brief 计算高度 */
    static int computeHeight(Node* x);

    Node* m_root = nullptr;
    Node* m_nil = nullptr;    ///< 哨兵节点

    Stats m_stats;
    double m_timeSum = 0.0;
};

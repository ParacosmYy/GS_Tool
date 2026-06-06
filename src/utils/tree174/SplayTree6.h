/**
 * @file SplayTree6.h
 * @brief 伸展树(zig/zig-zig/zig-zag旋转+split/join操作) — Splay Tree with Zig/Zig-Zig/Zig-Zag Rotations and Split/Join Operations
 *
 * 功能: 实现Splay Tree数据结构，支持zig/zig-zig/zig-zag自旋调整、
 *       split/join操作、范围查询和中序遍历。
 *
 * 协作: Treap6(树堆) / AVLTree7(AVL树) / RedBlackTree6(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Splay Tree(伸展树)
 */
class SplayTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
    };

    explicit SplayTree6(QObject *parent = nullptr);
    ~SplayTree6() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 查找键对应的值 */
    bool find(double key, double& value);

    /** @brief 按键分裂为两棵树 */
    void split(double key, SplayTree6& left, SplayTree6& right);

    /** @brief 合并另一棵树(所有key须小于本树最小key) */
    void join(SplayTree6& other);

    /** @brief 范围查询[keyLo, keyHi] */
    QVector<QPair<double, double>> rangeQuery(double keyLo, double keyHi);

    /** @brief 中序遍历 */
    QVector<QPair<double, double>> inOrder() const;

    bool isEmpty() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void splitCompleted(int leftSize, int rightSize);

private:
    /** @brief 树节点 */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node() = default;
        Node(double k, double v) : key(k), value(v) {}
    };

    /** @brief 右旋 */
    void rotateRight(Node* x);

    /** @brief 左旋 */
    void rotateLeft(Node* x);

    /** @brief Splay操作: 将节点旋转到根 */
    void splay(Node* x);

    /** @brief 查找节点(不splay) */
    Node* findNode(double key) const;

    /** @brief 获取子树最小节点 */
    static Node* subtreeMin(Node* t);

    /** @brief 获取子树最大节点 */
    static Node* subtreeMax(Node* t);

    /** @brief 中序遍历递归 */
    static void inOrderRec(Node* t, QVector<QPair<double, double>>& result);

    /** @brief 范围查询递归 */
    static void rangeRec(Node* t, double lo, double hi,
                          QVector<QPair<double, double>>& result);

    /** @brief 递归清除 */
    static void clearRec(Node* t);

    /** @brief 计算树高 */
    static int computeHeight(Node* t);

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
};

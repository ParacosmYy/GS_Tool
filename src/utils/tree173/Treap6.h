/**
 * @file Treap6.h
 * @brief 树堆(随机优先级插入+合并分裂删除) — Treap with Random Priority Insertion, Delete by Merge and Split/Merge Operations
 *
 * 功能: 实现Treap数据结构，支持随机优先级插入、split/merge删除、
 *       按键分裂、范围查询和中序遍历。
 *
 * 协作: CartesianTree5(笛卡尔树) / AVLTree7(AVL树) / RedBlackTree6(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Treap(树堆)
 */
class Treap6 : public QObject {
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

    explicit Treap6(QObject *parent = nullptr);
    ~Treap6() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键(通过split/merge) */
    bool remove(double key);

    /** @brief 查找键对应的值 */
    bool find(double key, double& value) const;

    /** @brief 按键分裂为两棵Treap */
    void split(double key, Treap6& left, Treap6& right);

    /** @brief 合并另一棵Treap(所有key必须小于本Treap最小key) */
    void merge(Treap6& other);

    /** @brief 范围查询[keyLo, keyHi] */
    QVector<QPair<double, double>> rangeQuery(double keyLo, double keyHi) const;

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
    /** @brief Treap节点 */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        double priority = 0.0;
        int size = 1;            ///< 子树大小
        Node* left = nullptr;
        Node* right = nullptr;

        Node() = default;
        Node(double k, double v, double p) : key(k), value(v), priority(p) {}
    };

    /** @brief 更新子树大小 */
    static void updateSize(Node* t);

    /** @brief 右旋 */
    static Node* rotateRight(Node* t);

    /** @brief 左旋 */
    static Node* rotateLeft(Node* t);

    /** @brief 内部插入(递归) */
    Node* insertRec(Node* t, double key, double value, double priority);

    /** @brief 内部分裂 */
    static void splitRec(Node* t, double key, Node*& left, Node*& right);

    /** @brief 内部合并 */
    static Node* mergeRec(Node* left, Node* right);

    /** @brief 查找 */
    static Node* findRec(Node* t, double key);

    /** @brief 范围查询 */
    static void rangeRec(Node* t, double lo, double hi,
                          QVector<QPair<double, double>>& result);

    /** @brief 中序遍历 */
    static void inOrderRec(Node* t, QVector<QPair<double, double>>& result);

    /** @brief 递归清除 */
    static void clearRec(Node* t);

    /** @brief 计算树高 */
    static int computeHeight(Node* t);

    /** @brief 生成随机优先级 */
    double randomPriority() const;

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;
    mutable quint64 m_rngState = 42;
};

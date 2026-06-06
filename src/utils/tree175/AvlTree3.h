/**
 * @file AvlTree3.h
 * @brief AVL平衡树(高度平衡旋转+select/rank序统计) — AVL Tree with Height-Balanced Rotations and Select/Rank Order Statistics
 *
 * 功能: 实现AVL自平衡二叉搜索树，支持LL/RR/LR/RL旋转调整、
 *       select(第k小)和rank(排名)序统计查询。
 *
 * 协作: SplayTree6(伸展树) / Treap6(树堆) / RedBlackTree6(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief AVL平衡二叉搜索树(带序统计)
 */
class AvlTree3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;          ///< 累计插入次数
        quint64 totalDeletes = 0;          ///< 累计删除次数
        quint64 totalQueries = 0;          ///< 累计查询次数
        int currentSize = 0;               ///< 当前元素数
        int treeHeight = 0;                ///< 当前树高
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit AvlTree3(QObject *parent = nullptr);
    ~AvlTree3() override;

    /** @brief 插入键值对 */
    bool insert(double key, double value);

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 查找键对应的值 */
    bool find(double key, double& value) const;

    /** @brief 选择第k小的元素(1-based) */
    bool select(int k, double& key, double& value) const;

    /** @brief 查询key的排名(1-based) */
    int rank(double key) const;

    /** @brief 范围查询[keyLo, keyHi] */
    QVector<QPair<double, double>> rangeQuery(double keyLo, double keyHi) const;

    /** @brief 中序遍历 */
    QVector<QPair<double, double>> inOrder() const;

    bool isEmpty() const;
    void clear();
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void insertCompleted(double key, int size);
    void deleteCompleted(double key);
    void rotationPerformed(const QString& type);

private:
    /** @brief AVL树节点(含子树大小) */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        int height = 1;     ///< 节点高度
        int subtreeSize = 1;///< 子树大小(用于序统计)
        Node* left = nullptr;
        Node* right = nullptr;

        Node() = default;
        Node(double k, double v) : key(k), value(v) {}
    };

    /** @brief 获取节点高度 */
    static int getHeight(Node* t);
    /** @brief 获取子树大小 */
    static int getSize(Node* t);
    /** @brief 计算平衡因子 */
    static int balanceFactor(Node* t);
    /** @brief 更新节点元数据 */
    static void updateMeta(Node* t);

    /** @brief LL型右旋 */
    Node* rotateRight(Node* y);
    /** @brief RR型左旋 */
    Node* rotateLeft(Node* x);
    /** @brief 平衡节点 */
    Node* balance(Node* t);

    /** @brief 递归插入 */
    Node* insertRec(Node* t, double key, double value, bool& inserted);
    /** @brief 递归删除 */
    Node* removeRec(Node* t, double key, bool& removed);
    /** @brief 取子树最小节点 */
    static Node* subtreeMin(Node* t);
    /** @brief 选择第k小 */
    static Node* selectRec(Node* t, int k);
    /** @brief 计算排名 */
    static int rankRec(Node* t, double key);
    /** @brief 中序遍历 */
    static void inOrderRec(Node* t, QVector<QPair<double, double>>& result);
    /** @brief 范围查询 */
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

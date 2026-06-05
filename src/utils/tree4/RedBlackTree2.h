/**
 * @file RedBlackTree2.h
 * @brief 红黑树(Red-Black Tree)增强版
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QVariant>

/**
 * @class RedBlackTree2
 * @brief 红黑树 — 支持顺序统计的平衡二叉搜索树
 *
 * 基于节点颜色标记的自平衡BST，支持O(log n)插入/删除/查找。
 * 增强版支持第k大查询、排名查询和范围统计。
 */
class RedBlackTree2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInserts = 0;       /**< 总插入数 */
        int totalRemoves = 0;       /**< 总删除数 */
        int totalSearches = 0;      /**< 总查找数 */
        int totalRotations = 0;     /**< 总旋转次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit RedBlackTree2(QObject* parent = nullptr);
    ~RedBlackTree2();

    /** @brief 插入键值对 */
    void insert(double key, const QVariant& value = QVariant());

    /** @brief 删除键 */
    bool remove(double key);

    /** @brief 查找键 */
    QVariant search(double key) const;

    /** @brief 是否包含键 */
    bool contains(double key) const;

    /** @brief 第k小元素的键 */
    double kth(int k) const;

    /** @brief 小于key的元素数量 */
    int rank(double key) const;

    /** @brief 范围内元素数量 */
    int rangeCount(double lo, double hi) const;

    /** @brief 中序遍历 */
    QVector<double> inOrderKeys() const;

    /** @brief 元素数量 */
    int size() const;

    /** @brief 树高度 */
    int height() const;

    /** @brief 验证红黑树性质 */
    bool verify() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 */
    void insertCompleted(double key);
    /** @brief 删除完成信号 */
    void removeCompleted(double key, bool success);

private:
    enum Color { Red, Black };

    struct Node {
        double key;
        QVariant value;
        Color color;
        Node* left;
        Node* right;
        Node* parent;
        int subtreeSize;
    };

    Node* m_nil;
    Node* m_root;
    int m_count;
    Stats m_stats;
    double m_timeSum;

    void leftRotate(Node* x);
    void rightRotate(Node* y);
    void insertFixup(Node* z);
    void deleteFixup(Node* x);
    void transplant(Node* u, Node* v);
    Node* treeMinimum(Node* x) const;
    void updateSize(Node* node);
    Node* findNode(double key) const;
    void deleteTree(Node* node);
    void inOrderHelper(Node* node, QVector<double>& result) const;
    int heightHelper(Node* node) const;
    bool verifyHelper(Node* node, int blackCount, int& pathBlackCount) const;
};

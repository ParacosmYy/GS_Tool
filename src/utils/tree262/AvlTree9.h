/**
 * @file AvlTree9.h
 * @brief AVL树(线程化节点O(1)后继前驱父指针重平衡高效迭代) — AVL Tree with Threaded Nodes for O(1) Successor/Predecessor and Parent-pointer Rebalancing for Efficient Iteration
 *
 * 功能: 实现AVL树(AVL tree)数据结构，采用线程化节点(threaded nodes)实现
 *       O(1)后继/前驱(successor/predecessor)查找，以及父指针重平衡
 *       (parent-pointer rebalancing)进行高效迭代。
 *
 * 协作: AA9(AA树) / RedBlackTree9(红黑树) / SplayTree8(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief AVL树(线程化节点O(1)后驱前驱父指针重平衡高效迭代)
 */
class AvlTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit AvlTree9(QObject *parent = nullptr);
    ~AvlTree9() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Find successor of a key (O(1) via threading) */
    int successor(int key) const;

    /** @brief Find predecessor of a key (O(1) via threading) */
    int predecessor(int key) const;

    /** @brief In-order traversal */
    QVector<int> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get node count */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeModified(int numNodes, int height, int rotations, double timeMs);

private:
    /** @brief AVL tree node with threading and parent pointer */
    struct Node {
        int key = 0;
        int balanceFactor = 0;  // height(right) - height(left)
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        Node* threadNext = nullptr;    // threaded successor
        Node* threadPrev = nullptr;    // threaded predecessor
        bool leftIsThread = false;     // left points to predecessor
        bool rightIsThread = false;    // right points to successor
    };

    Node* m_root = nullptr;
    int m_rotations = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Right rotation, returns new root of subtree */
    Node* rotateRight(Node* node);

    /** @brief Left rotation, returns new root of subtree */
    Node* rotateLeft(Node* node);

    /** @brief Rebalance from node upward using parent pointers */
    void rebalance(Node* node);

    /** @brief Update balance factor for a node */
    int updateBalance(Node* node) const;

    /** @brief Update thread links after insertion */
    void updateThreads(Node* newNode, Node* parent, bool isLeft);

    /** @brief Repair threads after rotation */
    void repairThreads(Node* node);

    /** @brief Find node by key */
    Node* findNode(int key) const;

    /** @brief Find minimum node in subtree */
    Node* findMin(Node* node) const;

    /** @brief Find maximum node in subtree */
    Node* findMax(Node* node) const;

    /** @brief In-order helper */
    void inOrderHelper(Node* node, QVector<int>& result) const;

    /** @brief Recursive height computation */
    int heightHelper(Node* node) const;

    /** @brief Delete all nodes */
    void clearHelper(Node* node);
};

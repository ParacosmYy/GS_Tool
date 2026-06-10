/**
 * @file SplayTree13.h
 * @brief 伸展树(手指搜索与动态手指定理保证的局部性感知访问模式) — Splay Tree with Finger Search and Dynamic Finger Theorem Guarantee for Locality-aware Access Patterns
 *
 * 功能: 实现伸展树(Splay tree)，采用手指搜索(finger search)
 *       与动态手指定理保证(dynamic finger theorem guarantee)实现局部性感知访问模式(locality-aware access patterns)。
 *
 * 协作: RedBlackTree16(红黑树) / AVLTree15(AVL树) / Treap12(树堆)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(手指搜索与动态手指定理保证)
 */
class SplayTree13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
        int numSplays = 0;
        int numRotations = 0;
    };

    explicit SplayTree13(QObject *parent = nullptr);
    ~SplayTree13() override;

    /** @brief Insert key and splay it to root */
    void insert(double key);

    /** @brief Remove key */
    void remove(double key);

    /** @brief Find key and splay it to root */
    bool contains(double key);

    /** @brief Finger search: find key closest to last accessed (dynamic finger) */
    double fingerSearch(double key);

    /** @brief Find minimum key */
    double minimum();

    /** @brief Find maximum key */
    double maximum();

    /** @brief Find successor of key */
    double successor(double key);

    /** @brief Find predecessor of key */
    double predecessor(double key);

    /** @brief In-order traversal */
    QVector<double> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of nodes */
    int size() const { return m_numNodes; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int numNodes, int height, double timeMs);
    void splayDone(int rotations);

private:
    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_finger = nullptr;   // Last accessed node (dynamic finger)
    int m_numNodes = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Rotate left around node x */
    void rotateLeft(Node* x);

    /** @brief Rotate right around node y */
    void rotateRight(Node* y);

    /** @brief Splay node x to root */
    void splay(Node* x);

    /** @brief Zig step (x is child of root) */
    void zig(Node* x);

    /** @brief Zig-zig step */
    void zigZig(Node* x);

    /** @brief Zig-zag step */
    void zigZag(Node* x);

    /** @brief Find node by key (no splay) */
    Node* findNode(double key) const;

    /** @brief Find subtree minimum */
    Node* subtreeMin(Node* x) const;

    /** @brief Find subtree maximum */
    Node* subtreeMax(Node* x) const;

    /** @brief Join two subtrees (all keys in left < all keys in right) */
    Node* join(Node* left, Node* right);

    /** @brief Split tree at key into (left, right) */
    void split(double key, Node*& left, Node*& right);

    /** @brief Recursive in-order */
    void inOrderRec(Node* x, QVector<double>& result) const;

    /** @brief Recursive height */
    int heightRec(Node* x) const;

    /** @brief Destroy subtree */
    void destroyTree(Node* x);

    /** @brief Set parent pointer helper */
    void setChild(Node* parent, Node* child, bool isLeft);
};

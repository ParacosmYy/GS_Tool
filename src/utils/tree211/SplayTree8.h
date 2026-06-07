/**
 * @file SplayTree8.h
 * @brief 伸展树(工作集伸展启发式+手指树混合实现顺序敏感O(1)摊还访问) — Splay Tree with Working-Set Splay Heuristic and Finger Tree Hybrid for Amortized O(1) Sequential Access
 *
 * 功能: 实现伸展树，支持工作集伸展启发式、
 *       手指树混合和摊还O(1)顺序访问优化。
 *
 * 协作: RedBlackTree11(红黑树) / BPlusTree8(B+树) / VanEmdeBoas5(vEB树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(工作集伸展启发式+手指树混合实现顺序敏感O(1)摊还访问)
 */
class SplayTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int totalSplays = 0;
        int sequentialHits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree8(QObject *parent = nullptr);
    ~SplayTree8() override;

    /** @brief Insert key with working-set splay */
    void insert(int key);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Search key (splays to root on access) */
    bool contains(int key);

    /** @brief Sequential access: get next key after current finger */
    int fingerNext();

    /** @brief Sequential access: get previous key before current finger */
    int fingerPrev();

    /** @brief Set finger position for sequential access */
    void setFinger(int key);

    /** @brief Get all elements in sorted order */
    QVector<int> inorder() const;

    /** @brief Get k-th smallest element (1-based) */
    int select(int k) const;

    /** @brief Count elements in range [lo, hi] */
    int rangeCount(int lo, int hi) const;

    /** @brief Get tree size */
    int size() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    /** @brief Node with access timestamp for working-set heuristic */
    struct Node {
        int key;
        int accessTime;    // Timestamp for working-set heuristic
        Node *left, *right, *parent;

        Node(int k) : key(k), accessTime(0),
            left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Node *m_root = nullptr;
    Node *m_finger = nullptr;   // Finger pointer for sequential access
    int m_timeCounter = 0;      // Global access time counter

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Standard zig rotation */
    void rotateLeft(Node *x);
    void rotateRight(Node *x);

    /** @brief Standard splay operation */
    void splay(Node *x);

    /** @brief Working-set splay: double-splay with depth check */
    void workingSetSplay(Node *x);

    /** @brief Find node by key */
    Node* search(int key) const;

    /** @brief Find minimum in subtree */
    Node* minimum(Node *x) const;

    /** @brief Find maximum in subtree */
    Node* maximum(Node *x) const;

    /** @brief Join two subtrees (all keys in s < all keys in t) */
    Node* join(Node *s, Node *t);

    /** @brief Split tree at key into (<=key, >key) */
    void split(int key, Node*& left, Node*& right);

    /** @brief Recursive inorder traversal */
    void inorderHelper(Node *x, QVector<int>& result) const;

    /** @brief Recursive delete */
    void deleteTree(Node *x);

    /** @brief Compute tree height */
    int heightHelper(Node *x) const;
};

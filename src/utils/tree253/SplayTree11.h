/**
 * @file SplayTree11.h
 * @brief 伸展树(手指搜索优化+半伸展降低摊还深度) — Splay Tree with Finger Search Optimization and Semi-Splay for Amortized Depth Reduction on Repeated Access Patterns
 *
 * 功能: 实现伸展树(Splay Tree)，支持手指搜索优化(finger search optimization)
 *       加速局部访问模式，半伸展(semi-splay)策略在非完全伸展时降低摊还深度，
 *       对重复访问模式(repeated access patterns)具有自适应性。
 *
 * 协作: RedBlackTree14(红黑树) / AVLTree7(AVL树) / ScapegoatTree9(替罪羊树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(手指搜索+半伸展优化)
 */
class SplayTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numSplays = 0;
        int numFingerHits = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree11(QObject *parent = nullptr);
    ~SplayTree11() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for key, splay to root if found */
    bool contains(int key);

    /** @brief Finger search: search near last accessed key */
    bool fingerSearch(int key);

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void splayCompleted(int key, int numRotations, double timeMs);

private:
    /** @brief Splay tree node */
    struct Node {
        int key = 0;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_finger = nullptr; // Last accessed node (finger)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Full splay: bring node to root */
    void splay(Node* x);

    /** @brief Semi-splay: partial splay for amortized depth reduction */
    void semiSplay(Node* x);

    /** @brief Left rotation */
    void rotateLeft(Node* x);

    /** @brief Right rotation */
    void rotateRight(Node* x);

    /** @brief Zig (single rotation with parent) */
    void zig(Node* x);

    /** @brief Zig-zig (same direction double rotation) */
    void zigZig(Node* x);

    /** @brief Zig-zag (opposite direction double rotation) */
    void zigZag(Node* x);

    /** @brief Find node by key */
    Node* findNode(int key) const;

    /** @brief Find minimum in subtree */
    Node* minimum(Node* x) const;

    /** @brief Find maximum in subtree */
    Node* maximum(Node* x) const;

    /** @brief In-order traversal */
    void inorder(Node* node, QVector<int>& result) const;

    /** @brief Compute height */
    int heightRec(Node* node) const;

    /** @brief Delete all nodes */
    void deleteTree(Node* node);
};

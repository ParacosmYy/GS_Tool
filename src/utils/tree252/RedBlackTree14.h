/**
 * @file RedBlackTree14.h
 * @brief 红黑树(范围提取批量删除+兄弟旋转优化再平衡) — Red-Black Tree with Bulk Delete via Range Extraction and Rebalancing with Sibling Rotation Optimization
 *
 * 功能: 实现红黑树(Red-Black Tree)，支持范围提取批量删除(range extraction
 *       bulk delete)一次性移除区间内所有节点，兄弟旋转优化(sibling rotation
 *       optimization)在删除再平衡时减少旋转次数。
 *
 * 协作: AVLTree7(AVL树) / ScapegoatTree9(替罪羊树) / VanEmdeBoas8(van Emde Boas树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(范围批量删除+兄弟旋转优化)
 */
class RedBlackTree14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int numInsertions = 0;
        int numDeletions = 0;
        int numBulkDeletes = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree14(QObject *parent = nullptr);
    ~RedBlackTree14() override;

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a single key */
    void remove(int key);

    /** @brief Bulk delete all keys in [lo, hi] range */
    void bulkDelete(int lo, int hi);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Clear all nodes */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bulkDeleteCompleted(int rangeLo, int rangeHi, int removed, double timeMs);

private:
    enum Color { Red, Black };

    /** @brief RB tree node */
    struct Node {
        int key = 0;
        Color color = Red;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_nil = nullptr;  // Sentinel node

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left rotation */
    void rotateLeft(Node* x);

    /** @brief Right rotation */
    void rotateRight(Node* y);

    /** @brief Sibling rotation optimization for delete fixup */
    void deleteFixup(Node* x);

    /** @brief Transplant subtree */
    void transplant(Node* u, Node* v);

    /** @brief Find minimum node in subtree */
    Node* minimum(Node* x) const;

    /** @brief Recursive insert fixup */
    void insertFixup(Node* z);

    /** @brief Sibling rotation: prefer single rotation over double */
    void siblingRotationOpt(Node* parent, bool isLeft);

    /** @brief Extract nodes in range, return as list */
    void extractRange(Node* node, int lo, int hi, QVector<Node*>& extracted);

    /** @brief In-order traversal */
    void inorder(Node* node, QVector<int>& result) const;

    /** @brief Compute height */
    int heightRec(Node* node) const;

    /** @brief Delete all nodes */
    void deleteTree(Node* node);
};

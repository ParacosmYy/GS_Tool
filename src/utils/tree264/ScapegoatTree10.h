/**
 * @file ScapegoatTree10.h
 * @brief 替罪羊树(权重平衡替罪羊查找子树重建摊还O(log n)重平衡) — Scapegoat Tree with Amortized O(log n) Rebalancing via Weight-balanced Scapegoat Finding and Subtree Rebuild
 *
 * 功能: 实现替罪羊树(Scapegoat tree)，采用权重平衡替罪羊查找(weight-balanced
 *       scapegoat finding)和子树重建(subtree rebuild)实现摊还O(log n)重平衡
 *       (amortized O(log n) rebalancing)。
 *
 * 协作: AvlTree9(AVL树) / RedBlackTree9(红黑树) / SplayTree8(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(权重平衡替罪羊查找子树重建摊还O(log n)重平衡)
 */
class ScapegoatTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        double alpha = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree10(double alpha = 0.7, QObject *parent = nullptr);
    ~ScapegoatTree10() override;

    /** @brief Insert a key-value pair */
    void insert(int key, double value);

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Search for key, returns value or NaN */
    double search(int key) const;

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> keys() const;

    /** @brief Get number of keys */
    int size() const;

    /** @brief Get tree height */
    int height() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numKeys, int height, int numRebuilds, double timeMs);

private:
    /** @brief Tree node */
    struct Node {
        int key;
        double value;
        Node* left = nullptr;
        Node* right = nullptr;
        int subtreeSize = 1;
    };

    double m_alpha;
    Node* m_root = nullptr;
    int m_size = 0;
    int m_maxSize = 0;  // Track max size since last rebuild (for delete rebalance)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get size of subtree */
    int nodeSize(Node* node) const { return node ? node->subtreeSize : 0; }

    /** @brief Update node size from children */
    void updateSize(Node* node);

    /** @brief Check if node is alpha-weight-balanced */
    bool isBalanced(Node* node) const;

    /** @brief Find scapegoat node on insertion path */
    Node* findScapegoat(Node* node, int key);

    /** @brief Flatten subtree into sorted array */
    void flatten(Node* node, QVector<Node*>& nodes);

    /** @brief Rebuild balanced subtree from sorted array */
    Node* rebuildBalanced(QVector<Node*>& nodes, int start, int end);

    /** @brief Rebuild subtree rooted at scapegoat */
    void rebuildSubtree(Node*& root, Node* scapegoat);

    /** @brief Insert recursively, returns (node, depth) */
    QPair<Node*, int> insertRec(Node* node, int key, double value, int depth);

    /** @brief Remove recursively */
    Node* removeRec(Node* node, int key);

    /** @brief Find minimum node */
    Node* findMin(Node* node) const;

    /** @brief Compute height */
    int computeHeight(Node* node) const;

    /** @brief Collect keys in-order */
    void inOrderKeys(Node* node, QVector<int>& result) const;

    /** @brief Delete all nodes */
    void clearTree(Node* node);

    /** @brief Node pointer to parent pointer (for rebuild) */
    struct PathEntry { Node** link; Node* node; };
    QVector<PathEntry> findPath(int key);
};

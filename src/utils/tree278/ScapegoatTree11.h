/**
 * @file ScapegoatTree11.h
 * @brief 替罪羊树(对数加权重建阈值与仅插入摊还保证的写优化二叉搜索树) — Scapegoat Tree with Log-weighted Rebuilding Threshold and Insertion-only Amortized Guarantee for Write-optimized BST
 *
 * 功能: 实现替罪羊树(Scapegoat tree)，采用对数加权重建阈值(log-weighted rebuilding threshold)
 *       与仅插入摊还保证(insertion-only amortized guarantee)实现写优化二叉搜索树(write-optimized BST)。
 *
 * 协作: AvlTree10(AVL树) / RedBlackTree12(红黑树) / BPlusTree13(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 替罪羊树(对数加权重建阈值与仅插入摊还保证)
 */
class ScapegoatTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numKeys = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRebuilds = 0;
        double alpha = 0.0;          // Balance parameter
        double avgProcessingTimeMs = 0.0;
    };

    explicit ScapegoatTree11(QObject *parent = nullptr);
    ~ScapegoatTree11() override;

    /** @brief Set balance parameter alpha (0.5..0.99) */
    void setAlpha(double alpha);

    /** @brief Insert a key-value pair */
    void insert(int key, double value);

    /** @brief Find value by key, returns NaN if not found */
    double find(int key) const;

    /** @brief Remove a key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get all keys in sorted order */
    QVector<int> allKeys() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of keys */
    int size() const;

    /** @brief Clear all data */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rebuildTriggered(int scapegoatKey, int subtreeSize, int treeHeight, double timeMs);
    void operationDone(const QString& op, int numKeys, int treeHeight, double timeMs);

private:
    double m_alpha = 0.75;  // Balance factor (weight-balanced threshold)

    /** @brief Tree node stored in array */
    struct Node {
        int key = 0;
        double value = 0.0;
        int left = -1;      // Index of left child
        int right = -1;     // Index of right child
        int size = 1;       // Subtree size
        bool active = true; // For soft-delete
    };

    QVector<Node> m_nodes;
    int m_root = -1;
    int m_maxSize = 0;     // Track max size since last rebuild (for deletion threshold)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate a new node, returns index */
    int allocNode(int key, double value);

    /** @brief Compute subtree size */
    int subtreeSize(int nodeIdx) const;

    /** @brief Compute tree height */
    int computeHeight(int nodeIdx) const;

    /** @brief Check if subtree is alpha-weight-balanced */
    bool isBalanced(int nodeIdx) const;

    /** @brief Find the scapegoat node (first unbalanced ancestor) */
    int findScapegoat(int nodeIdx, const QVector<int>& path) const;

    /** @brief Flatten subtree into sorted array */
    void flatten(int nodeIdx, QVector<QPair<int, double>>& entries) const;

    /** @brief Rebuild balanced BST from sorted entries */
    int rebuildBalanced(const QVector<QPair<int, double>>& entries, int lo, int hi);

    /** @brief Recursive insert helper, returns new root of subtree */
    int insertRec(int nodeIdx, int key, double value, QVector<int>& path);

    /** @brief Update subtree sizes up the path */
    void updateSizes(const QVector<int>& path);

    /** @brief In-order traversal */
    void inOrder(int nodeIdx, QVector<int>& keys) const;

    /** @brief Find node index for key */
    int findNode(int key) const;
};

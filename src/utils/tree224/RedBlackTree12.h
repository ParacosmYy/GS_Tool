/**
 * @file RedBlackTree12.h
 * @brief 红黑树(增强子树求和+逆秩查询百分位访问) — Red-Black Tree with Augmented Subtree Sum and Inverse Rank Query for Percentile-Based Access
 *
 * 功能: 实现红黑树，增强子树求和信息以支持O(log n)逆秩查询，
 *       实现基于百分位的快速元素访问。
 *
 * 协作: VanEmdeBoas6(vEB树) / ScapegoatTree7(替罪羊树) / BPlusTree9(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(子树求和+逆秩查询)
 */
class RedBlackTree12 : public QObject {
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

    explicit RedBlackTree12(QObject *parent = nullptr);
    ~RedBlackTree12() override;

    /** @brief Insert key with associated weight value */
    void insert(double key, double value = 1.0);

    /** @brief Remove key */
    bool remove(double key);

    /** @brief Check if key exists */
    bool contains(double key) const;

    /** @brief Find k-th smallest element (rank query) */
    double select(int k) const;

    /** @brief Inverse rank: find element at percentile p (0.0-1.0) */
    double percentile(double p) const;

    /** @brief Get rank of key (number of elements <= key) */
    int rank(double key) const;

    /** @brief Get total sum of all values */
    double totalSum() const;

    /** @brief Get sum of values with keys <= given key */
    double prefixSum(double key) const;

    /** @brief Get all keys in sorted order */
    QVector<double> keys() const;

    /** @brief Get number of nodes */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int size, double timeMs);

private:
    enum Color { RED, BLACK };

    struct RBNode {
        double key = 0.0;
        double value = 1.0;
        int subtreeSize = 1;
        double subtreeSum = 1.0;
        Color color = RED;
        RBNode* left = nullptr;
        RBNode* right = nullptr;
        RBNode* parent = nullptr;
    };

    RBNode* m_root = nullptr;
    RBNode* m_nil = nullptr;  // Sentinel node

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate new node */
    RBNode* allocNode(double key, double value);

    /** @brief Update augmented data (subtreeSize, subtreeSum) */
    void updateAugmented(RBNode* node);

    /** @brief Left rotation */
    void rotateLeft(RBNode* x);

    /** @brief Right rotation */
    void rotateRight(RBNode* y);

    /** @brief Fix tree after insertion */
    void insertFixup(RBNode* z);

    /** @brief Fix tree after deletion */
    void removeFixup(RBNode* x);

    /** @brief Transplant subtree */
    void transplant(RBNode* u, RBNode* v);

    /** @brief Find minimum node in subtree */
    RBNode* treeMinimum(RBNode* node) const;

    /** @brief Find node by key */
    RBNode* findNode(double key) const;

    /** @brief Recursive in-order key collection */
    void collectKeys(RBNode* node, QVector<double>& result) const;

    /** @brief Recursive tree height */
    int computeHeight(RBNode* node) const;

    /** @brief Recursive delete */
    void deleteTree(RBNode* node);
};

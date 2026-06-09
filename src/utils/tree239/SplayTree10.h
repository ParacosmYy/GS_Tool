/**
 * @file SplayTree10.h
 * @brief 伸展树(自顶向下zig-zig/zig-zag重构+工作集定理摊还O(log n)) — Splay Tree with Top-Down Zig-Zig/Zig-Zag Restructuring and Working-Set Theorem Amortized O(log n) Access
 *
 * 功能: 实现伸展树(Splay tree)，采用自顶向下zig-zig/zig-zag重构(top-down zig-zig/zig-zag
 *       restructuring)在访问时将节点展开至根，利用工作集定理(working-set theorem)保证
 *       摊还O(log n)访问复杂度(amortized O(log n) access)。
 *
 * 协作: RedBlackTree13(红黑树) / AVLTree9(AVL树) / VanEmdeBoas7(vEB树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 伸展树(自顶向下zig-zig/zig-zag重构+工作集定理摊还O(log n))
 */
class SplayTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Key-value pair */
    struct Entry {
        double key = 0.0;
        double value = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numSplayOps = 0;
        int numZigZig = 0;
        int numZigZag = 0;
        int numZig = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SplayTree10(QObject *parent = nullptr);
    ~SplayTree10() override;

    /** @brief Insert key-value pair, splay to root */
    void insert(double key, double value);

    /** @brief Remove key, splay parent to root */
    bool remove(double key);

    /** @brief Find value by key, splay to root */
    bool find(double key, double& value);

    /** @brief Get minimum key (splay to root) */
    Entry minimum();

    /** @brief Get maximum key (splay to root) */
    Entry maximum();

    /** @brief Split tree into (< key) and (>= key) */
    void split(double key, SplayTree10& left, SplayTree10& right);

    /** @brief Merge two trees (all keys in other >= all keys in this) */
    void merge(SplayTree10& other);

    /** @brief Get all entries in sorted order */
    QVector<Entry> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Check if tree is empty */
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void splayCompleted(double key, int rotations, double timeMs);
    void nodeInserted(double key);
    void nodeRemoved(double key, bool success);

private:
    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        double value = 0.0;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Top-down splay: bring key to root */
    Node* splay(Node* root, double key);

    /** @brief Right rotation */
    Node* rotateRight(Node* x);

    /** @brief Left rotation */
    Node* rotateLeft(Node* x);

    /** @brief In-order traversal helper */
    void inOrderRec(Node* n, QVector<Entry>& result) const;

    /** @brief Compute height recursively */
    int heightRec(Node* n) const;

    /** @brief Recursively destroy subtree */
    void destroyTree(Node* n);

    /** @brief Clone subtree (for split/merge) */
    Node* cloneTree(Node* n) const;
};

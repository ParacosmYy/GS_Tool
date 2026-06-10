/**
 * @file VanEmdeBoas9.h
 * @brief van Emde Boas树(聚类底层数组与摘要递归O(lg lg U)前驱后继查询) — van Emde Boas Tree with Clustered Bottom Arrays and Summary Recursion for O(lg lg U) Predecessor/Successor Queries
 *
 * 功能: 实现van Emde Boas树(van Emde Boas tree)，采用聚类底层数组(clustered bottom
 *       arrays)和摘要递归(summary recursion)实现O(lg lg U)前驱/后继查询
 *       (O(lg lg U) predecessor/successor queries)。
 *
 * 协作: AvlTree9(AVL树) / RedBlackTree9(红黑树) / BTree8(B树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief van Emde Boas树(聚类底层数组与摘要递归O(lg lg U)前驱后继查询)
 */
class VanEmdeBoas9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int universeSize = 0;
        int numElements = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas9(int universeSize = 65536, QObject *parent = nullptr);
    ~VanEmdeBoas9() override;

    /** @brief Insert a value into the vEB tree */
    void insert(int x);

    /** @brief Remove a value from the vEB tree */
    void remove(int x);

    /** @brief Check if value exists */
    bool contains(int x) const;

    /** @brief Find successor of x (smallest element >= x), or -1 */
    int successor(int x) const;

    /** @brief Find predecessor of x (largest element <= x), or -1 */
    int predecessor(int x) const;

    /** @brief Get minimum element, or -1 if empty */
    int minimum() const;

    /** @brief Get maximum element, or -1 if empty */
    int maximum() const;

    /** @brief Get number of elements */
    int size() const;

    /** @brief Get all elements in sorted order */
    QVector<int> elements() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numElements, int universeSize, double timeMs);

private:
    /** @brief vEB node with clustered bottom and summary */
    struct Node {
        int min = -1;       // Min stored directly (not in clusters)
        int max = -1;       // Max stored directly
        int universe = 2;   // Universe size for this node
        Node* summary = nullptr;
        QVector<Node*> clusters;

        int high(int x) const { return x >> (lowerBits() / 2 + lowerBits() % 2); }
        int low(int x) const { return x & ((1 << lowerBits()) - 1); }
        int index(int h, int l) const { return (h << lowerBits()) | l; }
        int lowerBits() const { return universe / 2; }
        int numClusters() const { return qMax(1, 1 << (universe - universe / 2)); }
    };

    Node* m_root = nullptr;
    int m_universe = 0;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Create a vEB node with given universe size */
    Node* createNode(int u) const;

    /** @brief Recursively insert into vEB node */
    void insertRec(Node*& node, int x, int u);

    /** @brief Recursively remove from vEB node */
    void removeRec(Node*& node, int x, int u);

    /** @brief Recursively check membership */
    bool containsRec(Node* node, int x, int u) const;

    /** @brief Recursively find successor */
    int successorRec(Node* node, int x, int u) const;

    /** @brief Recursively find predecessor */
    int predecessorRec(Node* node, int x, int u) const;

    /** @brief Collect elements in order */
    void collectElements(Node* node, int base, int u, QVector<int>& result) const;

    /** @brief Recursively delete node tree */
    void deleteNode(Node* node);

    /** @brief Compute tree height (recursion depth) */
    int computeHeight(int u) const;

    /** @brief Round universe to next power of 2 */
    static int nextPower2(int n);
};

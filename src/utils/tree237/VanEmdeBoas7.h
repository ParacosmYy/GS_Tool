/**
 * @file VanEmdeBoas7.h
 * @brief van Emde Boas树(簇递归结构+自底向上最小值查找O(log log U)) — van Emde Boas Tree with Cluster-Based Recursive Structure and Bottom-Up Minimum Finding for O(log log U) Operations
 *
 * 功能: 实现van Emde Boas树(vEB tree)，采用簇递归结构(cluster-based recursive structure)
 *       和自底向上最小值查找(bottom-up minimum finding)，支持O(log log U)时间的
 *       插入、删除、查找、前驱和后继操作。
 *
 * 协作: ScapegoatTree8(替罪羊树) / RedBlackTree6(红黑树) / BPlusTree10(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief van Emde Boas树(簇递归结构+自底向上最小值查找)
 */
class VanEmdeBoas7 : public QObject {
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

    explicit VanEmdeBoas7(QObject *parent = nullptr);
    ~VanEmdeBoas7() override;

    /** @brief Initialize with universe size U (must be power of 2) */
    bool init(int universeSize);

    /** @brief Insert key with value */
    void insert(int key, double value = 0.0);

    /** @brief Remove key */
    void remove(int key);

    /** @brief Check if key exists */
    bool contains(int key) const;

    /** @brief Get value for key (NaN if not found) */
    double value(int key) const;

    /** @brief Find successor of key (-1 if none) */
    int successor(int key) const;

    /** @brief Find predecessor of key (-1 if none) */
    int predecessor(int key) const;

    /** @brief Get minimum key (-1 if empty) */
    int minimum() const;

    /** @brief Get maximum key (-1 if empty) */
    int maximum() const;

    /** @brief Get all stored keys (sorted) */
    QVector<int> allKeys() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void keyInserted(int key);
    void keyRemoved(int key);

private:
    /** @brief vEB node with cluster-based recursive structure */
    struct VEBNode {
        int min = -1;           // min key stored directly
        int max = -1;           // max key stored directly
        double minValue = 0.0;  // value for min key
        VEBNode* summary = nullptr;
        QVector<VEBNode*> clusters;
        QVector<double> values; // values for non-min keys in this node's range

        int universe;           // universe size of this node
        int sqrtU;              // high bits count
    };

    VEBNode* m_root = nullptr;
    int m_universe = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Create a vEB node for given universe */
    VEBNode* createNode(int u) const;

    /** @brief Destroy a vEB node recursively */
    void destroyNode(VEBNode* node);

    /** @brief High bits of x (cluster index) */
    int high(int x, int u) const;

    /** @brief Low bits of x (position within cluster) */
    int low(int x, int u) const;

    /** @brief Combine high and low into index */
    int index(int high, int low, int u) const;

    /** @brief Recursive insert */
    void insertRec(VEBNode* node, int key, double value);

    /** @brief Recursive remove */
    void removeRec(VEBNode* node, int key);

    /** @brief Recursive contains */
    bool containsRec(VEBNode* node, int key) const;

    /** @brief Recursive successor */
    int successorRec(VEBNode* node, int key) const;

    /** @brief Recursive predecessor */
    int predecessorRec(VEBNode* node, int key) const;

    /** @brief Collect keys in-order */
    void collectKeys(VEBNode* node, int offset, QVector<int>& keys) const;

    /** @brief Compute sqrt ceiling */
    static int sqrtCeil(int u);
    static int sqrtFloor(int u);
};

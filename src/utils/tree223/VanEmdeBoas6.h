/**
 * @file VanEmdeBoas6.h
 * @brief vEB树(哈希簇存储+自底向上递归查找下一个稀疏宇宙) — Van Emde Boas Tree with Hash-Based Cluster Storage and Bottom-Up Recursive Find-Next for Sparse Universe
 *
 * 功能: 实现van Emde Boas树数据结构，使用哈希表存储簇以节省空间，
 *       自底向上递归find-next操作优化稀疏宇宙场景。
 *
 * 协作: ScapegoatTree7(替罪羊树) / RedBlackTree5(红黑树) / BPlusTree9(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>

/**
 * @brief vEB树(哈希簇+自底向上查找)
 */
class VanEmdeBoas6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int universeSize = 0;
        int numElements = 0;
        int treeHeight = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit VanEmdeBoas6(QObject *parent = nullptr);
    ~VanEmdeBoas6() override;

    /** @brief Initialize vEB tree with universe size (power of 2) */
    void init(int universeSize);

    /** @brief Insert a key */
    void insert(int key);

    /** @brief Remove a key */
    bool remove(int key);

    /** @brief Check if key is a member */
    bool contains(int key) const;

    /** @brief Find successor of key */
    int findNext(int key) const;

    /** @brief Find predecessor of key */
    int findPrev(int key) const;

    /** @brief Get minimum element */
    int minimum() const;

    /** @brief Get maximum element */
    int maximum() const;

    /** @brief Get all elements in sorted order */
    QVector<int> elements() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key, double timeMs);

private:
    struct VEBNode {
        int min = -1;    // -1 = empty
        int max = -1;
        int universe = 2;
        int sqrtU = 1;
        VEBNode* summary = nullptr;
        QHash<int, VEBNode*> cluster;

        ~VEBNode() {
            delete summary;
            qDeleteAll(cluster);
        }

        int high(int x) const { return x / sqrtU; }
        int low(int x) const { return x % sqrtU; }
        int index(int h, int l) const { return h * sqrtU + l; }
    };

    VEBNode* m_root = nullptr;
    int m_universe = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Allocate vEB node for given universe */
    VEBNode* allocNode(int u);

    /** @brief Recursive insert into vEB subtree */
    void insertRec(VEBNode* node, int key);

    /** @brief Recursive remove from vEB subtree */
    bool removeRec(VEBNode* node, int key);

    /** @brief Recursive membership test */
    bool containsRec(const VEBNode* node, int key) const;

    /** @brief Bottom-up find-next (successor) */
    int findNextRec(const VEBNode* node, int key) const;

    /** @brief Bottom-up find-prev (predecessor) */
    int findPrevRec(const VEBNode* node, int key) const;

    /** @brief Recursive element collection */
    void collectElements(const VEBNode* node, int base,
                          QVector<int>& result) const;

    /** @brief Compute tree height */
    int computeHeight(const VEBNode* node) const;

    /** @brief Count active clusters */
    int countClusters(const VEBNode* node) const;
};

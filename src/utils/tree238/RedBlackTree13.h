/**
 * @file RedBlackTree13.h
 * @brief 红黑树(区间增强节点重叠查询+批量插入色翻优化) — Red-Black Tree with Interval-Augmented Nodes for Overlap Queries and Color-Flip Optimization on Bulk Insert
 *
 * 功能: 实现红黑树(Red-Black tree)扩展变体，采用区间增强节点(interval-augmented nodes)
 *       支持区间重叠查询(interval overlap queries)，在批量插入时使用色翻优化(color-flip
 *       optimization)减少旋转次数，维护最大子树区间边界(max subtree interval bounds)。
 *
 * 协作: VanEmdeBoas7(vEB树) / ScapegoatTree8(替罪羊树) / BPlusTree10(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(区间增强节点重叠查询+批量插入色翻优化)
 */
class RedBlackTree13 : public QObject {
    Q_OBJECT

public:
    /** @brief Interval [low, high] with associated value */
    struct Interval {
        double low = 0.0;
        double high = 0.0;
        double value = 0.0;
    };

    /** @brief Overlap query result */
    struct OverlapResult {
        Interval interval;
        bool exact = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        int numColorFlips = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit RedBlackTree13(QObject *parent = nullptr);
    ~RedBlackTree13() override;

    /** @brief Insert single interval */
    void insert(const Interval& iv);

    /** @brief Bulk insert intervals with color-flip optimization */
    void bulkInsert(const QVector<Interval>& intervals);

    /** @brief Remove interval by exact match */
    bool remove(const Interval& iv);

    /** @brief Find all intervals overlapping [low, high] */
    QVector<OverlapResult> queryOverlap(double low, double high) const;

    /** @brief Check if any interval contains point */
    bool containsPoint(double point) const;

    /** @brief Get all intervals in sorted order */
    QVector<Interval> allIntervals() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeInserted(double low, double high);
    void nodeRemoved(double low, double high);
    void bulkInsertCompleted(int count, int flips, double timeMs);

private:
    /** @brief RB node color */
    enum Color { Red = 0, Black = 1 };

    /** @brief Tree node with interval augmentation */
    struct Node {
        Interval iv;
        Color color = Red;
        double maxHigh = 0.0;  // max high in subtree
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_nil = nullptr;  // sentinel

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Create sentinel nil node */
    void initNil();

    /** @brief Update augmented max-high field */
    void updateMaxHigh(Node* n);

    /** @brief Update max-high from node to root */
    void fixMaxHigh(Node* n);

    /** @brief Left rotation */
    void rotateLeft(Node* x);

    /** @brief Right rotation */
    void rotateRight(Node* y);

    /** @brief Fix RB properties after insert */
    void insertFixup(Node* z);

    /** @brief Fix RB properties after delete */
    void removeFixup(Node* x);

    /** @brief Transplant subtree */
    void transplant(Node* u, Node* v);

    /** @brief Find minimum node in subtree */
    Node* treeMinimum(Node* x) const;

    /** @brief Recursive overlap query */
    void queryOverlapRec(Node* n, double low, double high,
                         QVector<OverlapResult>& results) const;

    /** @brief In-order traversal */
    void inOrderCollect(Node* n, QVector<Interval>& out) const;

    /** @brief Recursive destroy */
    void destroyTree(Node* n);

    /** @brief Bulk insert sort-and-build approach */
    Node* buildSorted(const QVector<Interval>& sorted, int lo, int hi, Color color);

    /** @brief Compute tree height */
    int height(Node* n) const;
};

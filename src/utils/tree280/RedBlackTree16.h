/**
 * @file RedBlackTree16.h
 * @brief 红黑树(左倾变体与迭代插入颜色翻转的简化平衡树维护) — Red-Black Tree with Left-Leaning Variant and Iterative Insertion with Color Flips for Simplified Balanced Tree Maintenance
 *
 * 功能: 实现红黑树(red-black tree)，采用左倾变体(left-leaning variant)
 *       与迭代插入颜色翻转(iterative insertion with color flips)实现简化平衡树维护(simplified balanced tree maintenance)。
 *
 * 协作: VanEmdeBoas10(vEB树) / ScapegoatTree11(替罪羊树) / BPlusTree13(B+树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 红黑树(左倾变体与迭代插入颜色翻转)
 */
class RedBlackTree16 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int treeHeight = 0;
        double avgProcessingTimeMs = 0.0;
        int numRotations = 0;
        int numColorFlips = 0;
    };

    explicit RedBlackTree16(QObject *parent = nullptr);
    ~RedBlackTree16() override;

    /** @brief Insert a key (iterative with color flips) */
    void insert(double key);

    /** @brief Remove a key */
    void remove(double key);

    /** @brief Check if key exists */
    bool contains(double key) const;

    /** @brief Find minimum key */
    double minimum() const;

    /** @brief Find maximum key */
    double maximum() const;

    /** @brief Find successor of key */
    double successor(double key) const;

    /** @brief Find predecessor of key */
    double predecessor(double key) const;

    /** @brief In-order traversal */
    QVector<double> inOrder() const;

    /** @brief Get tree height */
    int height() const;

    /** @brief Get number of nodes */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int numNodes, int height, double timeMs);
    void rebalanceDone(int rotations, int colorFlips);

private:
    /** @brief Node color */
    enum Color { RED, BLACK };

    /** @brief Tree node */
    struct Node {
        double key = 0.0;
        Color color = RED;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* m_root = nullptr;
    Node* m_nil = nullptr;    // Sentinel node (black)
    int m_numNodes = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Left-leaning fix: ensure red links lean left */
    void fixLean(Node* node);

    /** @brief Rotate left */
    void rotateLeft(Node* x);

    /** @brief Rotate right */
    void rotateRight(Node* y);

    /** @brief Color flip (parent + children) */
    void colorFlip(Node* h);

    /** @brief Fixup after insert (iterative) */
    void insertFixup(Node* z);

    /** @brief Fixup after delete */
    void deleteFixup(Node* x);

    /** @brief Transplant subtree */
    void transplant(Node* u, Node* v);

    /** @brief Find node by key */
    Node* findNode(double key) const;

    /** @brief Tree minimum from subtree */
    Node* treeMinimum(Node* x) const;

    /** @brief Tree maximum from subtree */
    Node* treeMaximum(Node* x) const;

    /** @brief Recursive in-order */
    void inOrderRec(Node* x, QVector<double>& result) const;

    /** @brief Recursive height */
    int heightRec(Node* x) const;

    /** @brief Recursive delete all nodes */
    void destroyTree(Node* x);

    /** @brief Is node red (nil = black) */
    bool isRed(Node* x) const { return x != nullptr && x->color == RED; }
};

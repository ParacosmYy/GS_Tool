/**
 * @file WeightBalancedTree5.h
 * @brief 权重平衡树(大小重平衡+秩/选择+join2拼接) — Weight-Balanced Tree with Size-Based Rebalancing, Rank/Selection and Join2 Concatenation
 *
 * 功能: 实现权重平衡树，支持节点大小维护、δ-γ平衡条件、
 *       秩查询/选择操作和join2拼接合并。
 *
 * 协作: AVLTree4(AVL树) / RedBlackTree9(红黑树) / ScapegoatTree5(替罪羊树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 权重平衡树(大小重平衡+秩/选择)
 */
class WeightBalancedTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int numNodes = 0;
        int treeHeight = 0;
        int numRotations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WeightBalancedTree5(QObject *parent = nullptr);
    ~WeightBalancedTree5() override;

    void setDelta(double delta);
    void setGamma(double gamma);

    /** @brief 插入键值 */
    void insert(int key);

    /** @brief 删除键值 */
    void remove(int key);

    /** @brief 查找键值 */
    bool contains(int key) const;

    /** @brief 秩查询: 小于key的元素个数 */
    int rank(int key) const;

    /** @brief 选择: 第k小的元素(1-indexed) */
    int select(int k) const;

    /** @brief join2: 拼接两棵树(本树所有key < other所有key) */
    void join2(WeightBalancedTree5& other);

    /** @brief 中序遍历 */
    QVector<int> inorder() const;

    int size() const;
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int key);

private:
    /** @brief Tree node with size */
    struct Node {
        int key = 0;
        int weight = 1; // subtree size
        Node* left = nullptr;
        Node* right = nullptr;
    };

    Node* m_root = nullptr;
    double m_delta = 0.28;   // balance parameter (delta > 0, typically 0.28)
    double m_gamma = 0.42;   // gamma >= delta / (1 - delta)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Get node weight */
    int w(Node* n) const { return n ? n->weight : 0; }

    /** @brief Update node weight */
    void updateWeight(Node* n);

    /** @brief Check and rebalance */
    Node* balance(Node* n);

    /** @brief Single rotation left */
    Node* rotateLeft(Node* n);

    /** @brief Single rotation right */
    Node* rotateRight(Node* n);

    /** @brief Double rotation left-right */
    Node* rotateLeftRight(Node* n);

    /** @brief Double rotation right-left */
    Node* rotateRightLeft(Node* n);

    /** @brief Insert helper */
    Node* insertNode(Node* n, int key);

    /** @brief Remove helper */
    Node* removeNode(Node* n, int key);

    /** @brief Rank helper */
    int rankHelper(Node* n, int key) const;

    /** @brief Select helper */
    int selectHelper(Node* n, int k) const;

    /** @brief Join2 helper */
    Node* join2Nodes(Node* left, Node* right);

    /** @brief In-order helper */
    void inorderHelper(Node* n, QVector<int>& result) const;

    /** @brief Delete subtree */
    void deleteTree(Node* n);
};

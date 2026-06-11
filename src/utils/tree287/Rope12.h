/**
 * @file Rope12.h
 * @brief 绳索数据结构(权重平衡树与分裂-拼接优化的撤销支持文本编辑) — Rope with Weight-balanced Tree and Split-concat Optimization for Efficient Text Editing with Undo Support
 *
 * 功能: 实现绳索数据结构(Rope)，采用权重平衡树(weight-balanced tree)
 *       与分裂-拼接优化(split-concat optimization)实现撤销支持高效文本编辑(efficient text editing with undo support)。
 *
 * 协作: BTree10(B树) / AVLTree9(AVL树) / Trie8(Trie树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索数据结构(权重平衡树与分裂-拼接优化的撤销支持文本编辑)
 */
class Rope12 : public QObject {
    Q_OBJECT

public:
    /** @brief Edit operation for undo */
    struct EditOp {
        enum Type { Insert, Remove };
        Type type;
        int position = 0;
        QString text;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numNodes = 0;
        int ropeLength = 0;
        int undoStackSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope12(int leafCapacity = 64, QObject *parent = nullptr);
    ~Rope12() override;

    void setLeafCapacity(int cap);
    void setMaxUndoLevels(int levels);

    /** @brief Build rope from string */
    void build(const QString& text);

    /** @brief Insert text at position */
    void insert(int pos, const QString& text);

    /** @brief Remove range [pos, pos+len) */
    void remove(int pos, int len);

    /** @brief Get character at index */
    QChar at(int index) const;

    /** @brief Get substring [pos, pos+len) */
    QString mid(int pos, int len) const;

    /** @brief Get full text */
    QString toString() const;

    /** @brief Concatenate another rope */
    void concat(Rope12& other);

    /** @brief Split at position, return right half */
    Rope12* split(int pos);

    /** @brief Undo last operation */
    bool undo();

    int length() const { return m_length; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void editDone(const QString& opType, int pos, double timeMs);

private:
    int m_leafCap = 64;
    int m_maxUndo = 100;
    int m_length = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    static constexpr int NULL_NODE = -1;

    /** @brief Rope node */
    struct RopeNode {
        int weight = 0;           // Length of left subtree (or leaf chars)
        int left = NULL_NODE;     // Left child index
        int right = NULL_NODE;    // Right child index
        int parent = NULL_NODE;
        QString leafData;         // Non-empty for leaf nodes
        bool isLeaf = true;
    };

    QVector<RopeNode> m_nodes;
    QVector<int> m_freeList;
    int m_root = NULL_NODE;
    QVector<EditOp> m_undoStack;

    /** @brief Allocate node */
    int allocNode(bool isLeaf);

    /** @brief Free node */
    void freeNode(int idx);

    /** @brief Compute total length of subtree */
    int subtreeLen(int nodeIdx) const;

    /** @brief Rebalance weight-balanced tree at node */
    int rebalance(int nodeIdx);

    /** @brief Get balance factor */
    double balanceRatio(int nodeIdx) const;

    /** @brief Rotate left */
    int rotateLeft(int nodeIdx);

    /** @brief Rotate right */
    int rotateRight(int nodeIdx);

    /** @brief Recursive in-order traversal to build string */
    void toStringHelper(int nodeIdx, QString& result) const;

    /** @brief Find character at global index */
    QChar charAt(int nodeIdx, int index) const;

    /** @brief Build balanced rope from scratch */
    int buildBalanced(const QString& text, int start, int end);
};

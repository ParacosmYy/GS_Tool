/**
 * @file Rope5.h
 * @brief 绳索数据结构(手指树平衡拼接+子串反转+编辑距离) — Rope with Balanced Concatenation via Finger Tree, Substring Reversal and Edit Distance Support
 *
 * 功能: 实现Rope数据结构，支持手指树平衡拼接、
 *       子串反转、编辑距离计算和高效字符串操作。
 *
 * 协作: SuffixTree5(后缀树) / Trie5(字典树) / BWT5(Burrows-Wheeler变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 绳索数据结构(手指树平衡+编辑距离)
 */
class Rope5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int ropeLength = 0;
        int nodeCount = 0;
        int depth = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope5(QObject *parent = nullptr);
    ~Rope5() override;

    /** @brief Build rope from string */
    void build(const QString& str);

    /** @brief Concatenate with another rope (balanced via finger tree) */
    void concat(const Rope5& other);

    /** @brief Insert string at position */
    void insert(int pos, const QString& str);

    /** @brief Delete substring [pos, pos+len) */
    void remove(int pos, int len);

    /** @brief Get character at position */
    QChar at(int pos) const;

    /** @brief Extract substring [pos, pos+len) */
    QString substring(int pos, int len) const;

    /** @brief Reverse substring [pos, pos+len) */
    void reverse(int pos, int len);

    /** @brief Compute edit distance to another string/rope */
    int editDistance(const QString& other) const;

    /** @brief Compute edit distance with path reconstruction */
    QVector<QString> editPath(const QString& other) const;

    /** @brief Convert entire rope to string */
    QString toString() const;

    /** @brief Length of rope */
    int length() const;

    /** @brief Rebalance the rope */
    void rebalance();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int ropeLen, double timeMs);

private:
    /** @brief Rope node: leaf or internal */
    struct Node {
        int weight = 0;       // length of left subtree (or string for leaf)
        QString data;         // non-empty for leaf nodes
        Node* left = nullptr;
        Node* right = nullptr;
        bool isLeaf = true;

        ~Node() { delete left; delete right; }
    };

    Node* m_root = nullptr;
    int m_leafMax = 64; // max characters per leaf

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Create a leaf node from string */
    Node* makeLeaf(const QString& s) const;

    /** @brief Create an internal node */
    Node* makeInternal(Node* l, Node* r) const;

    /** @brief Compute weight of a node */
    static int nodeWeight(const Node* n);

    /** @brief Split rope at position into two */
    void splitAt(Node* node, int pos, Node*& left, Node*& right) const;

    /** @brief Concatenate two nodes with balancing */
    Node* concatNodes(Node* a, Node* b) const;

    /** @brief Collect all characters via in-order traversal */
    void collectChars(const Node* n, QString& out) const;

    /** @brief Collect at position */
    QChar charAt(const Node* n, int pos) const;

    /** @brief Compute depth of rope */
    static int depth(const Node* n);

    /** @brief Rebuild rope balanced from flat string */
    Node* buildBalanced(const QString& s, int start, int len) const;

    /** @brief Reverse helper */
    void reverseHelper(Node* n, int pos, int len, const QString& rev);

    /** @brief Count nodes */
    static int countNodes(const Node* n);
};

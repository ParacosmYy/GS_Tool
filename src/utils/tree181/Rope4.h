/**
 * @file Rope4.h
 * @brief Rope数据结构(叶子分裂/拼接平衡+子串提取+字符统计) — Rope Data Structure with Leaf-split/Concat Balancing, Substring Extraction and Character Statistics
 *
 * 功能: 实现Rope数据结构，支持高效字符串拼接、分裂、子串提取、
 *       叶子节点分裂/合并平衡和字符频率统计。
 *
 * 协作: SuffixTree4(后缀树) / Trie4(字典树) / SuffixArray4(后缀数组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Rope数据结构(叶子分裂/拼接平衡)
 */
class Rope4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int totalLength = 0;
        int numNodes = 0;
        int numLeaves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope4(QObject *parent = nullptr);
    ~Rope4() override;

    /** @brief 从字符串构建Rope */
    void build(const QString& text);

    /** @brief 拼接另一个Rope */
    void concat(const Rope4& other);

    /** @brief 在指定位置分裂，返回右半部分 */
    Rope4* split(int pos);

    /** @brief 提取子串 [start, start+length) */
    QString substring(int start, int length) const;

    /** @brief 获取指定位置字符 */
    QChar charAt(int index) const;

    /** @brief 在指定位置插入字符串 */
    void insert(int pos, const QString& text);

    /** @brief 删除 [start, start+length) 范围字符 */
    void remove(int start, int length);

    /** @brief 获取完整字符串 */
    QString toString() const;

    /** @brief 字符频率统计 */
    QVector<int> charFrequency() const;

    /** @brief 字符串长度 */
    int length() const;

    /** @brief 检查是否为空 */
    bool isEmpty() const;

    /** @brief 重新平衡Rope */
    void rebalance();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length);

private:
    /** @brief Rope节点 */
    struct Node {
        int weight = 0;           ///< Leaf: string length; Internal: left subtree total
        QString data;             ///< Non-empty only for leaf nodes
        Node* left = nullptr;
        Node* right = nullptr;

        bool isLeaf() const { return left == nullptr && right == nullptr; }
    };

    Node* m_root = nullptr;
    int m_leafSize = 64; ///< Maximum characters per leaf before split

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build balanced rope from string */
    Node* buildRecursive(const QString& text, int start, int end);

    /** @brief Compute weight of subtree */
    int computeWeight(Node* node) const;

    /** @brief Collect all text via in-order traversal */
    void collectText(Node* node, QString& result) const;

    /** @brief Extract substring recursively */
    void substringHelper(Node* node, int start, int length,
                          QString& result, int& consumed) const;

    /** @brief Flatten tree to leaves for rebalancing */
    void collectLeaves(Node* node, QVector<QString>& leaves) const;

    /** @brief Build balanced tree from leaves */
    Node* buildFromLeaves(const QVector<QString>& leaves, int start, int end);

    /** @brief Delete subtree */
    void deleteTree(Node* node);

    /** @brief Count nodes */
    void countNodes(Node* node, int& total, int& leafCount) const;

    /** @brief Char frequency helper */
    void charFreqHelper(Node* node, QVector<int>& freq) const;

    /** @brief Flatten to a list of leaf strings */
    void flattenLeaves(Node* node, QVector<QString>& out) const;

    /** @brief Split node at position */
    Node* splitNode(Node* node, int pos, Node*& rightPart);
};

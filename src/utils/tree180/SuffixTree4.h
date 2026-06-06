/**
 * @file SuffixTree4.h
 * @brief 后缀树(Ukkonen算法+活跃点+隐式后缀链接+模式搜索) — Suffix Tree via Ukkonen's Algorithm with Active Point, Implicit Suffix Links and Pattern Search
 *
 * 功能: 实现Ukkonen在线后缀树构建算法，支持活跃点三元组、
 *       隐式后缀链接、O(n)线性构建和模式搜索/最长重复子串。
 *
 * 协作: SuffixArray4(后缀数组) / Trie4(字典树) / AhoCorasick4(AC自动机)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 后缀树(Ukkonen算法)
 */
class SuffixTree4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        int textLength = 0;
        int numNodes = 0;
        int numEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SuffixTree4(QObject *parent = nullptr);
    ~SuffixTree4() override;

    /** @brief 构建后缀树 */
    void build(const QString& text);

    /** @brief 模式搜索: 返回所有匹配起始位置 */
    QVector<int> search(const QString& pattern) const;

    /** @brief 最长重复子串 */
    QString longestRepeatedSubstring() const;

    /** @brief 最长公共子串(与另一文本) */
    QString longestCommonSubstring(const QString& other) const;

    /** @brief 统计模式出现次数 */
    int countOccurrences(const QString& pattern) const;

    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int textLength, int numNodes);

private:
    /** @brief Tree edge */
    struct Edge {
        int start = 0;    ///< Start index in text
        int end = 0;      ///< End index (can be INT_MAX for open edges)
        int child = -1;   ///< Destination node
    };

    /** @brief Tree node */
    struct Node {
        int suffixLink = 0;       ///< Suffix link
        QVector<int> children;    ///< Child node indices (indexed by char)
        Node() : suffixLink(0) {}
    };

    QString m_text;
    QVector<Node> m_nodes;
    int m_alphabetSize = 128; // ASCII

    /** @brief Active point (node, edge, length) */
    int m_activeNode = 0;
    int m_activeEdge = 0;
    int m_activeLen = 0;
    int m_remaining = 0;
    int m_pos = 0;       ///< Current position in text
    int m_endPos = 0;    ///< Global end position (grows)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Map character to alphabet index */
    int charIndex(QChar c) const;

    /** @brief Get edge length */
    int edgeLen(int edgeEnd) const;

    /** @brief Create new node */
    int newNode();

    /** @brief Walk down from active point */
    bool walkDown(int node, int edgeStart, int edgeEnd);

    /** @brief Extend suffix tree at position pos */
    void extend(int pos);

    /** @brief Find edge from node with given character */
    int findEdge(int node, int chIdx) const;

    /** @brief Collect all leaf positions below node */
    void collectLeaves(int node, QVector<int>& positions) const;

    /** @brief Find deepest internal node with >=2 leaves */
    void deepestInternal(int node, int depth, int& bestDepth,
                          int& bestNode) const;
};

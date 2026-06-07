/**
 * @file SuffixTree5.h
 * @brief 广义后缀树(多串后缀链接+最长公共子串) — Generalized Suffix Tree for Multiple Strings with Suffix Links and Longest Common Substring
 *
 * 功能: 实现广义后缀树，支持多字符串插入、
 *       后缀链接遍历、最长公共子串查找和模式匹配。
 *
 * 协作: Trie5(字典树) / AhoCorasick4(AC自动机) / BWT5(Burrows-Wheeler变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 广义后缀树(多串后缀链接+LCS)
 */
class SuffixTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSearches = 0;
        int numStrings = 0;
        int totalLength = 0;
        int numNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SuffixTree5(QObject *parent = nullptr);
    ~SuffixTree5() override;

    /** @brief Insert a string into the generalized suffix tree */
    void insert(const QString& str);

    /** @brief Build tree from multiple strings at once */
    void build(const QVector<QString>& strings);

    /** @brief Check if pattern exists in any string */
    bool contains(const QString& pattern) const;

    /** @brief Find all occurrences of pattern */
    QVector<int> findAll(const QString& pattern) const;

    /** @brief Find longest common substring among all strings */
    QString longestCommonSubstring() const;

    /** @brief Find longest repeated substring */
    QString longestRepeatedSubstring() const;

    /** @brief Count distinct substrings */
    int countDistinctSubstrings() const;

    int numStrings() const { return m_strings.size(); }
    int numNodes() const { return m_nodes.size(); }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void searchCompleted(int occurrences, double timeMs);

private:
    /** @brief Tree edge: [start, end) in concatenated string */
    struct Edge {
        int start = -1;
        int end = -1;
        int child = -1;   // target node
    };

    struct Node {
        QVector<Edge> children;  // indexed by char
        int suffixLink = 0;
        int stringId = -1;       // which string owns this
    };

    QVector<Node> m_nodes;
    QString m_text;              // concatenated with separators
    QVector<int> m_sepPositions; // separator positions
    QVector<QString> m_strings;

    // Ukkonen's algorithm state
    int m_activeNode = 0;
    int m_activeEdge = 0;
    int m_activeLen = 0;
    int m_remainder = 0;
    int m_pos = 0;               // current position in text

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Extend suffix tree by one character (Ukkonen) */
    void extend(int pos);

    /** @brief Get character at position in concatenated text */
    QChar charAt(int pos) const;

    /** @brief Walk down from node following edge */
    int walkDown(int node, int pos) const;

    /** @brief Find length of edge */
    int edgeLen(int edge) const;
};

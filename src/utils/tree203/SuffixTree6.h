/**
 * @file SuffixTree6.h
 * @brief 压缩后缀树(后缀链接导航+Euler游历LCA+模式匹配) — Compressed Suffix Tree with Suffix Link Navigation, LCA via Euler Tour and Pattern Matching
 *
 * 功能: 实现压缩后缀树(Ukkonen算法)，支持后缀链接导航、
 *       Euler游历LCA查询和快速模式匹配。
 *
 * 协作: Trie8(字典树) / AVLTree9(AVL树) / SuffixArray7(后缀数组)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 压缩后缀树(后缀链接导航+Euler游历LCA+模式匹配)
 */
class SuffixTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int stringLength = 0;
        int nodeCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SuffixTree6(QObject *parent = nullptr);
    ~SuffixTree6() override;

    /** @brief Build suffix tree from string using Ukkonen's algorithm */
    void build(const QString& text);

    /** @brief Search for pattern, returns all start positions */
    QVector<int> search(const QString& pattern) const;

    /** @brief Count occurrences of pattern */
    int countOccurrences(const QString& pattern) const;

    /** @brief Find longest repeated substring */
    QString longestRepeatedSubstring() const;

    /** @brief Build Euler tour for LCA queries */
    void buildEulerTour();

    /** @brief LCA query via Euler tour + sparse table */
    int lca(int nodeA, int nodeB) const;

    /** @brief Navigate suffix links from node */
    int suffixLink(int node) const;

    int nodeCount() const;
    bool isEmpty() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeBuilt(int length, int nodes, double timeMs);

private:
    /** @brief Tree edge: [start, end) in text */
    struct Edge {
        int start = -1;
        int end = -1;
        int child = -1;
    };

    /** @brief Tree node */
    struct Node {
        int suffixLink = 0;
        int parent = -1;
        QVector<Edge> children;    // indexed by character offset
        int depth = 0;             // string depth
    };

    QVector<Node> m_nodes;
    QString m_text;
    int m_alphabetSize = 256;

    // Euler tour structures
    QVector<int> m_euler;          // node sequence
    QVector<int> m_eulerDepth;     // depth at each step
    QVector<int> m_firstOccur;     // first occurrence of node in Euler
    QVector<QVector<int>> m_sparseTable;  // for RMQ/LCA

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Ukkonen's active point */
    struct ActivePoint {
        int node = 0;
        int edge = 0;
        int length = 0;
    };

    /** @brief Extend suffix tree by one character */
    void extend(int position);

    /** @brief Get character at position */
    QChar charAt(int pos) const;

    /** @brief Build sparse table for RMQ */
    void buildSparseTable();

    /** @brief RMQ query on Euler depth array */
    int rmq(int i, int j) const;
};

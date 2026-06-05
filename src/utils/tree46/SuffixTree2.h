/**
 * @file SuffixTree2.h
 * @brief 后缀树2 — Ukkonen在线构建+LCP
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SuffixTree2 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalBuilds = 0;
        int totalQueries = 0;
        int totalStringLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SuffixTree2(QObject* parent = nullptr);
    ~SuffixTree2();

    void build(const QVector<int>& data);
    bool contains(const QVector<int>& pattern) const;
    int countOccurrences(const QVector<int>& pattern) const;
    QVector<int> longestRepeat() const;
    QVector<int> longestCommonSubstring(const QVector<int>& other) const;
    QVector<int> lcpArray() const;

    int length() const { return m_length; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int length, int nodes);

private:
    struct Node {
        int start, end, suffixLink;
        QMap<int, int> children;
    };

    QVector<Node> m_nodes;
    QVector<int> m_data;
    int m_length = 0;
    int m_root = 0;
    int m_activeNode = 0;
    int m_activeEdge = 0;
    int m_activeLen = 0;
    int m_remaining = 0;
    int m_position = -1;

    void extend(int position);
    int edgeLength(int node) const;
    bool walkDown(int node);
    int newNode(int start, int end);
    void destroyTree();

    Stats m_stats;
    double m_timeSum = 0.0;
};

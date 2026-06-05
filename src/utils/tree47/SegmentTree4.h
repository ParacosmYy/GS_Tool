/**
 * @file SegmentTree4.h
 * @brief 线段树4 — 可持久化+主席树
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class SegmentTree4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalUpdates = 0;
        int totalQueries = 0;
        int totalVersions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SegmentTree4(QObject* parent = nullptr);
    ~SegmentTree4();

    void build(const QVector<double>& data);
    int update(int version, int pos, double value);
    double query(int version, int lo, int hi) const;
    double queryKth(int version, int k) const;
    int count(int version, double low, double high) const;

    int currentVersion() const { return m_versions.size() - 1; }
    int numVersions() const { return m_versions.size(); }
    int size() const { return m_n; }
    QVector<double> versionData(int version) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void versionCreated(int version, int pos);

private:
    struct Node {
        double sum;
        int left, right;
    };

    int m_n = 0;
    QVector<Node> m_nodes;
    QVector<int> m_roots;
    QVector<int> m_versions;

    int buildTree(int lo, int hi, const QVector<double>& data);
    int updateTree(int node, int lo, int hi, int pos, double value);
    double queryTree(int node, int lo, int hi, int ql, int qr) const;
    int newNode(double sum, int left, int right);
    void destroyTree();

    Stats m_stats;
    double m_timeSum = 0.0;
};

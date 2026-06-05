#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MaxClique4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit MaxClique4(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> solve();
    int cliqueSize() const { return m_cliqueSize; }
    bool isClique(const QVector<int>& set) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int size);
private:
    int m_n = 0; int m_cliqueSize = 0;
    QVector<QVector<int>> m_adj;
    void expand(QVector<int>& current, QVector<int>& candidates, QVector<int>& best);
    QVector<int> colorSort(const QVector<int>& candidates);
    Stats m_stats; double m_timeSum = 0.0;
};

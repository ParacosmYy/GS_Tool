#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class TopologicalSort3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSorts = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit TopologicalSort3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> sort();
    bool hasCycle() const { return m_hasCycle; }
    QVector<QVector<int>> allTopologicalSorts();
    int numSorts() const { return m_numSorts; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void sortCompleted(int n, bool dag);
private:
    int m_n = 0; bool m_hasCycle = false; int m_numSorts = 0;
    QVector<QVector<int>> m_adj; QVector<int> m_inDegree;
    void allSortsDFS(QVector<int>& result, QVector<bool>& visited, QVector<int>& inDeg);
    Stats m_stats; double m_timeSum = 0.0;
};

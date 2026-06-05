#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Biconnected3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit Biconnected3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<QVector<QPair<int,int>>> findBiconnectedComponents();
    QVector<int> articulationPoints() const { return m_articPoints; }
    int numBiconnectedComponents() const { return m_numComp; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void searchCompleted(int components, int articPoints);
private:
    int m_n = 0; int m_numComp = 0;
    QVector<QVector<int>> m_adj; QVector<int> m_articPoints;
    void dfs(int u, int parent, QVector<int>& disc, QVector<int>& low,
             QVector<QPair<int,int>>& stk, QVector<QVector<QPair<int,int>>>& result,
             int& time);
    Stats m_stats; double m_timeSum = 0.0;
};

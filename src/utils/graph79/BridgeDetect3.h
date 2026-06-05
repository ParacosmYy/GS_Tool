#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BridgeDetect3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearches = 0; int totalEdges = 0; double avgProcessingTimeMs = 0.0; };
    explicit BridgeDetect3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<QPair<int,int>> findBridges();
    int numBridges() const { return m_numBridges; }
    bool hasBridge() const { return m_numBridges > 0; }
    QVector<QPair<int,int>> bridgeEdges() const { return m_bridges; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void searchCompleted(int bridges);
private:
    int m_n = 0; int m_numBridges = 0;
    QVector<QVector<int>> m_adj; QVector<QPair<int,int>> m_bridges;
    void dfs(int u, int parent, QVector<int>& disc, QVector<int>& low, int& time);
    Stats m_stats; double m_timeSum = 0.0;
};

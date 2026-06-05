#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class StronglyConnected4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSearches = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit StronglyConnected4(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<QVector<int>> findSCCs();
    QVector<int> condensationDAG() const;
    bool isStronglyConnected() const;
    int numSCCs() const { return m_sccs.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void searchCompleted(int sccs);
private:
    int m_n = 0;
    QVector<QVector<int>> m_adj; QVector<QVector<int>> m_sccs;
    void tarjanSCC(int u, QVector<int>& disc, QVector<int>& low,
                   QVector<bool>& onStack, QVector<int>& stack, int& timer);
    Stats m_stats; double m_timeSum = 0.0;
};

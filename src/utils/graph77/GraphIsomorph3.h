#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GraphIsomorph3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalChecks = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit GraphIsomorph3(QObject* parent = nullptr);
    void setGraph1(int n, const QVector<QPair<int,int>>& edges);
    void setGraph2(int m, const QVector<QPair<int,int>>& edges);
    bool isIsomorphic();
    QVector<int> mapping() const { return m_mapping; }
    bool hasIsomorphism() const { return m_found; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void checkCompleted(bool iso, int n);
private:
    int m_n1 = 0; int m_n2 = 0; bool m_found = false;
    QVector<QVector<int>> m_adj1; QVector<QVector<int>> m_adj2;
    QVector<int> m_mapping;
    QVector<int> computeInvariant(const QVector<QVector<int>>& adj);
    bool backtrackMatch(QVector<int>& map, QVector<bool>& used, int depth);
    Stats m_stats; double m_timeSum = 0.0;
};

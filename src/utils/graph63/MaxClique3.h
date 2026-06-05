#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

class MaxClique3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit MaxClique3(QObject* parent = nullptr);
    void setGraph(int n, const QVector<QPair<int,int>>& edges);
    QVector<int> solve();
    int cliqueSize() const { return m_clique.size(); }
    bool isClique(const QVector<int>& vertices) const;
    int vertices() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solved(int size);
private:
    int m_n = 0;
    QVector<QVector<int>> m_adj; QVector<int> m_clique;
    void expand(QVector<int>& current, QVector<int>& candidates);
    int colorBound(const QVector<int>& candidates) const;
    Stats m_stats; double m_timeSum = 0.0;
};

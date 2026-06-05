#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class IndependentSet3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalVertices = 0; double avgProcessingTimeMs = 0.0; };
    explicit IndependentSet3(QObject* parent = nullptr);
    void setVertexCount(int n);
    void addEdge(int u, int v);
    QVector<int> solve();
    int setSize() const { return m_setSize; }
    bool isIndependent(const QVector<int>& set) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int size);
private:
    int m_n = 0; int m_setSize = 0;
    QVector<QVector<int>> m_adj;
    void branchAndBound(QVector<int>& current, QVector<int>& best, QVector<bool>& used, int idx);
    int upperBound(const QVector<int>& current, const QVector<bool>& used) const;
    Stats m_stats; double m_timeSum = 0.0;
};

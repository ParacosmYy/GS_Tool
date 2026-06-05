#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PrioritySearchTree2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit PrioritySearchTree2(QObject* parent = nullptr);
    void insert(double x, double y, double priority, int id);
    QVector<int> query(double xLo, double xHi, double yMax) const;
    int topPriority(double xLo, double xHi, double yMax) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void pointInserted(double x, double y, double priority);
private:
    int m_size = 0;
    struct PSTNode { double x, y, priority; int id; PSTNode *left, *right; double maxY; };
    PSTNode* m_root = nullptr;
    PSTNode* insertNode(PSTNode* n, double x, double y, double p, int id);
    void queryNode(PSTNode* n, double xLo, double xHi, double yMax, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class IntervalTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit IntervalTree4(QObject* parent = nullptr);
    void insert(double lo, double hi, int id);
    QVector<int> queryPoint(double point) const;
    QVector<int> queryInterval(double lo, double hi) const;
    void remove(int id);
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void intervalInserted(double lo, double hi);
private:
    int m_size = 0;
    struct Node { double lo, hi, maxHi; int id; Node *left, *right; int height; };
    Node* m_root = nullptr;
    Node* insertNode(Node* n, double lo, double hi, int id);
    Node* balance(Node* n);
    int height(Node* n) const;
    Node* rotateLeft(Node* n);
    Node* rotateRight(Node* n);
    void queryPointNode(Node* n, double pt, QVector<int>& res) const;
    void queryIntervalNode(Node* n, double lo, double hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class QuadTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit QuadTree3(QObject* parent = nullptr);
    void setBounds(double xMin, double yMin, double xMax, double yMax);
    void setMaxCapacity(int cap);
    void insert(double x, double y, int id);
    QVector<int> queryRange(double xMin, double yMin, double xMax, double yMax) const;
    QVector<int> nearestNeighbor(double x, double y, int k) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void pointInserted(double x, double y);
private:
    int m_capacity = 4; int m_size = 0;
    double m_xMin, m_yMin, m_xMax, m_yMax;
    struct QNode;
    QNode* m_root = nullptr;
    QNode* insertNode(QNode* n, double x, double y, int id, double xMin, double yMin, double xMax, double yMax);
    void queryRangeNode(QNode* n, double xMin, double yMin, double xMax, double yMax, QVector<int>& res) const;
    void collectPoints(QNode* n, QVector<QPair<double,double>>& pts, QVector<int>& ids) const;
    Stats m_stats; double m_timeSum = 0.0;
};

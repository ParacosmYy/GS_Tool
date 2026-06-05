#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class RangeTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit RangeTree3(QObject* parent = nullptr);
    void setDimensions(int dim);
    void build(const QVector<QVector<double>>& points);
    QVector<int> query(const QVector<double>& lo, const QVector<double>& hi) const;
    int count(const QVector<double>& lo, const QVector<double>& hi) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeBuilt(int points, int dimensions);
    void queryCompleted(int results);
private:
    int m_dim = 2; int m_size = 0;
    struct RTNode;
    RTNode* m_root = nullptr;
    RTNode* buildTree(QVector<QPair<QVector<double>,int>>& pts, int depth);
    void queryNode(RTNode* n, const QVector<double>& lo, const QVector<double>& hi, QVector<int>& res, int depth) const;
    Stats m_stats; double m_timeSum = 0.0;
};

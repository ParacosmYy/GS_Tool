#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KDTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit KDTree4(QObject* parent = nullptr);
    void setDimensions(int dim);
    void build(const QVector<QVector<double>>& points);
    QVector<int> nearestNeighbor(const QVector<double>& query) const;
    QVector<int> kNearest(const QVector<double>& query, int k) const;
    QVector<int> rangeSearch(const QVector<double>& lo, const QVector<double>& hi) const;
    int size() const { return m_size; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeBuilt(int points);
private:
    int m_dim = 2; int m_size = 0;
    struct KdNode { QVector<double> pt; int id; KdNode *left, *right; int axis; };
    KdNode* m_root = nullptr;
    KdNode* buildTree(QVector<QPair<QVector<double>,int>>& pts, int depth, int lo, int hi);
    void nnSearch(KdNode* n, const QVector<double>& q, double& bestDist, int& bestId, int depth) const;
    void knnSearch(KdNode* n, const QVector<double>& q, int k, QVector<QPair<double,int>>& best, int depth) const;
    void rangeSearchNode(KdNode* n, const QVector<double>& lo, const QVector<double>& hi, QVector<int>& res) const;
    Stats m_stats; double m_timeSum = 0.0;
};

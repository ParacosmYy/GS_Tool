#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class PersistentTree2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInsertions = 0; int totalDeletions = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit PersistentTree2(QObject* parent = nullptr);
    int insert(int version, double key, int value);
    int remove(int version, double key);
    int find(int version, double key) const;
    double selectKth(int version, int k) const;
    int rank(int version, double key) const;
    int numVersions() const;
    int versionSize(int version) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void versionCreated(int version);
private:
    struct Node { double key; int value; int left, right, size; };
    int copyNode(int idx);
    int insertRec(int idx, double key, int value);
    int removeRec(int idx, double key);
    QVector<Node> m_pool; QVector<int> m_roots;
    Stats m_stats; double m_timeSum = 0.0;
};

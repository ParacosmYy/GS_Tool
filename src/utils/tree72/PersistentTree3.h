#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PersistentTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalInserts = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit PersistentTree3(QObject* parent = nullptr);
    int insert(double key, int value, int version);
    void remove(double key, int version);
    bool contains(double key, int version) const;
    int rank(double key, int version) const;
    int numVersions() const { return m_versions.size(); }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void versionCreated(int version);
private:
    struct PNode { double key; int val, count; PNode *left, *right; };
    QVector<PNode*> m_versions;
    PNode* insertNode(PNode* n, double key, int val);
    PNode* cloneNode(PNode* n);
    Stats m_stats; double m_timeSum = 0.0;
};

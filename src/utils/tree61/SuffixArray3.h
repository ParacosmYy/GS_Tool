#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SuffixArray3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit SuffixArray3(QObject* parent = nullptr);
    void build(const QVector<int>& data);
    QVector<int> query(const QVector<int>& pattern) const;
    int occurrenceCount(const QVector<int>& pattern) const;
    int longestRepeat() const;
    QVector<int> suffixArray() const { return m_sa; }
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void buildCompleted(int n);
    void queryCompleted(int matches);
private:
    int m_n = 0;
    QVector<int> m_sa; QVector<int> m_rank; QVector<int> m_lcp;
    void buildSA(const QVector<int>& data);
    void buildLCP(const QVector<int>& data);
    int compare(int suf, const QVector<int>& pat) const;
    Stats m_stats; double m_timeSum = 0.0;
};

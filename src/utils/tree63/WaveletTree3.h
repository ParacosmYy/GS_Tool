#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class WaveletTree3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalBuilds = 0; int totalQueries = 0; double avgProcessingTimeMs = 0.0; };
    explicit WaveletTree3(QObject* parent = nullptr);
    void setAlphabetSize(int sigma);
    void build(const QVector<int>& data);
    int rank(int symbol, int pos) const;
    int select(int symbol, int k) const;
    int access(int pos) const;
    QVector<int> rangeFreq(int lo, int hi, int a, int b) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void treeBuilt(int n, int sigma);
private:
    int m_n = 0; int m_sigma = 256;
    struct WTNode { QVector<int> b; WTNode *left, *right; int lo, hi; };
    WTNode* m_root = nullptr;
    WTNode* buildNode(const QVector<int>& data, int lo, int hi);
    int rankNode(WTNode* n, int symbol, int pos, int lo, int hi) const;
    int selectNode(WTNode* n, int symbol, int k, int lo, int hi) const;
    Stats m_stats; double m_timeSum = 0.0;
};

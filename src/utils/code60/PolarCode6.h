#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PolarCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolarCode6(QObject* parent = nullptr);
    void setBlockLength(int n);
    void setInfoLength(int k);
    void setListSize(int l);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& llr);
    int blockLength() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int listSize, double metric);
private:
    int m_n = 128; int m_k = 64; int m_listSize = 4;
    QVector<int> m_frozenSet;
    void designFrozenSet();
    double scDecode(const QVector<double>& llr, QVector<int>& bits);
    Stats m_stats; double m_timeSum = 0.0;
};

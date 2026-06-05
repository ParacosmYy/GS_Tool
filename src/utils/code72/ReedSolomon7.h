#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ReedSolomon7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit ReedSolomon7(QObject* parent = nullptr);
    void setFieldOrder(int m);
    void setNumDataSymbols(int k);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int numCorrectable() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int errors);
private:
    int m_m = 8; int m_k = 223;
    int gfMul(int a, int b) const;
    int gfInv(int a) const;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GrayCode4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit GrayCode4(QObject* parent = nullptr);
    void setBits(int bits);
    int encode(int value) const;
    int decode(int gray) const;
    QVector<int> generateSequence() const;
    int hammingDistance(int a, int b) const;
    int bits() const { return m_bits; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int value, int gray);
private:
    int m_bits = 8;
    Stats m_stats; double m_timeSum = 0.0;
};

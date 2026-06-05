#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GrayCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit GrayCode5(QObject* parent = nullptr);
    void setBitWidth(int bits);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& gray);
    int encodeSingle(int value) const;
    int decodeSingle(int gray) const;
    int hammingDistance(int a, int b) const;
    QVector<int> generateCode() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int count, int bitWidth);
private:
    int m_bits = 8;
    Stats m_stats; double m_timeSum = 0.0;
};

/**
 * @file SpinalCode2.h
 * @brief Spinal code enhanced - hash-based encoding/sequential decoder
 */
#pragma once
#include <QObject>
#include <QVector>
class SpinalCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpinalCode2(QObject* parent = nullptr);
    void setSpineLength(int length);
    void setSymbolSize(int bits);
    void setNumPasses(int passes);
    QVector<int> encode(const QVector<int>& message) const;
    QVector<int> decode(const QVector<double>& softSymbols, int messageLength);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeComplete(int symbols);
    void decodeComplete(int bits);
private:
    int m_spineLen = 16; int m_symbolSize = 4; int m_numPasses = 8;
    Stats m_stats; double m_timeSum = 0.0;
};

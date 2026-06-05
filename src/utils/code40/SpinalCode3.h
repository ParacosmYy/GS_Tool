/**
 * @file SpinalCode3.h
 * @brief 脊码3 — 自适应编码率+尾比特归零
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class SpinalCode3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalSymbols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpinalCode3(QObject* parent = nullptr);

    void setSeed(quint32 seed);
    void setSpineLength(int length);
    QVector<int> encode(const QVector<int>& message, int numPasses = 4);
    QVector<int> decode(const QVector<int>& received, int beamWidth = 16,
                        int messageLength = -1);

    int spineLength() const { return m_spineLength; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int bits, int symbols);
    void decodeCompleted(int messageLength, bool success);

private:
    quint32 m_seed = 0x12345678;
    int m_spineLength = 32;
    int m_k = 4;
    int m_precision = 8;

    quint32 spineFunction(quint32 spineValue, quint32 bits) const;
    int rngSymbol(quint32 spineValue, int symbolIndex) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

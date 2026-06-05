/**
 * @file PolarCode4.h
 * @brief 极化码4 — CRC辅助SCL+自适应列表大小
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class PolarCode4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalBitsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolarCode4(QObject* parent = nullptr);

    void setParameters(int n, int k, int crcLength = 8, int maxListSize = 32);
    QVector<int> encode(const QVector<int>& infoBits);
    QVector<int> decode(const QVector<double>& llr);

    int blockLength() const { return m_n; }
    int infoLength() const { return m_k; }
    int listSize() const { return m_listSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int n, int k);
    void decodeCompleted(bool crcPass, int listUsed);

private:
    int m_n = 256;
    int m_k = 128;
    int m_crcLen = 8;
    int m_maxListSize = 32;
    int m_listSize = 32;
    QVector<int> m_frozenSet;
    QVector<int> m_infoSet;
    QVector<int> m_crcPoly;

    void generateFrozenSet();
    quint32 crcCompute(const QVector<int>& bits) const;
    void sortByReliability(QVector<QPair<double,int>>& reliabilities);

    Stats m_stats;
    double m_timeSum = 0.0;
};

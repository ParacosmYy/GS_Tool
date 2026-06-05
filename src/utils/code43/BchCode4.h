/**
 * @file BchCode4.h
 * @brief BCH码4 — 多项式欧几里得+Berlekamp-Massey
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class BchCode4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalErrorsCorrected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BchCode4(QObject* parent = nullptr);

    void setParameters(int n, int k, int t);
    QVector<int> encode(const QVector<int>& message);
    QVector<int> decode(const QVector<int>& received);
    int correctableErrors() const { return m_t; }

    int codeLength() const { return m_n; }
    int dataLength() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int n, int k);
    void decodeCompleted(int errorsCorrected);

private:
    int m_n = 511;
    int m_k = 502;
    int m_t = 2;
    int m_m = 9;
    QVector<int> m_generatorPoly;
    QVector<int> m_alphaTable;
    QVector<int> m_indexTable;

    void generateTables();
    int gfMul(int a, int b) const;
    int gfPow(int a, int n) const;
    QVector<int> gfPolyMod(const QVector<int>& dividend,
                            const QVector<int>& divisor) const;
    QVector<int> berlekampMassey(const QVector<int>& syndrome) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

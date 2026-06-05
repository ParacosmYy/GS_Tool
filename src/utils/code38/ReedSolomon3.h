/**
 * @file ReedSolomon3.h
 * @brief Reed-Solomon编解码器3 — 频域编解码+Chien搜索
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ReedSolomon3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalErrorsCorrected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ReedSolomon3(QObject* parent = nullptr);

    void setParameters(int n, int k, int m = 8);
    QVector<int> encode(const QVector<int>& message);
    QVector<int> decode(const QVector<int>& received);
    int correctableErrors() const { return m_t; }
    bool isCodeword(const QVector<int>& data) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int n, int k);
    void decodeCompleted(int errorsCorrected);

private:
    int m_n = 255;
    int m_k = 223;
    int m_m = 8;
    int m_t = 16;
    QVector<int> m_generatorPoly;
    QVector<int> m_alphaTable;
    QVector<int> m_indexTable;

    void generateTables();
    int gfMul(int a, int b) const;
    int gfInv(int a) const;
    QVector<int> gfPolyMul(const QVector<int>& a, const QVector<int>& b) const;
    QVector<int> gfPolyDiv(const QVector<int>& dividend, const QVector<int>& divisor) const;
    QVector<int> calcSyndromes(const QVector<int>& received) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

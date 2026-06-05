/**
 * @file ConvolutionalCode4.h
 * @brief 卷积码4 — 软判决Viterbi+回溯
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ConvolutionalCode4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalBitsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConvolutionalCode4(QObject* parent = nullptr);

    void setGenerator(const QVector<int>& generators, int constraintLength);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decodeSoft(const QVector<double>& softBits);
    QVector<int> decodeHard(const QVector<int>& hardBits);
    void terminate();

    int constraintLength() const { return m_K; }
    int numGenerators() const { return m_nGenerators; }
    double codingRate() const { return 1.0 / m_nGenerators; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inputBits, int outputBits);
    void decodeCompleted(int bits, double metric);

private:
    int m_K = 7;
    int m_nGenerators = 2;
    QVector<int> m_generators;
    int m_numStates = 64;
    QVector<QVector<int>> m_nextState;
    QVector<QVector<int>> m_output;
    bool m_tablesBuilt = false;

    void buildTables();
    int butterflyTransition(int state, int input) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file ConvolutionalCode5.h
 * @brief 卷积码5 — 打孔卷积+速率兼容
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class ConvolutionalCode5 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalBitsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConvolutionalCode5(QObject* parent = nullptr);

    void setGenerators(const QVector<int>& generators, int K);
    void setPuncturingPattern(const QVector<int>& pattern);
    void setRate(double rate);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);
    QVector<int> depuncture(const QVector<double>& softBits) const;

    double codeRate() const;
    int constraintLength() const { return m_K; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inputBits, int outputBits);

private:
    int m_K = 7;
    QVector<int> m_generators;
    QVector<int> m_puncturePattern;
    int m_puncturePeriod = 2;
    int m_numStates = 64;
    QVector<QVector<int>> m_nextState;
    QVector<QVector<int>> m_output;
    bool m_tablesBuilt = false;

    void buildTables();

    Stats m_stats;
    double m_timeSum = 0.0;
};

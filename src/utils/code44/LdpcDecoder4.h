/**
 * @file LdpcDecoder4.h
 * @brief LDPC解码器4 — 分层置信传播+早期终止
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class LdpcDecoder4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalDecodes = 0;
        int totalIterations = 0;
        int totalCodewords = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit LdpcDecoder4(QObject* parent = nullptr);

    void setParityMatrix(const QVector<QVector<int>>& H);
    void setMaxIterations(int maxIter);
    QVector<int> decode(const QVector<double>& llr);
    QVector<int> decodeLayered(const QVector<double>& llr);
    bool checkSyndrome(const QVector<int>& codeword) const;

    int codeLength() const { return m_n; }
    int dataLength() const { return m_k; }
    int maxIterations() const { return m_maxIterations; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int iterations, bool converged);

private:
    int m_n = 0;
    int m_k = 0;
    int m_maxIterations = 50;
    QVector<QVector<int>> m_H;
    QVector<QVector<int>> m_checkToVar;
    QVector<QVector<int>> m_varToCheck;

    void buildGraph();

    Stats m_stats;
    double m_timeSum = 0.0;
};

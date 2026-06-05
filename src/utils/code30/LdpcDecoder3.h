/**
 * @file LdpcDecoder3.h
 * @brief LDPC解码增强 — 最小和/分层调度/早起终止/量化消息
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class LdpcDecoder3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; double avgIterations = 0.0; };
    explicit LdpcDecoder3(QObject* parent = nullptr);
    void setParityMatrix(const QVector<QVector<int>>& H);
    void setMaxIterations(int maxIter);
    void setScalingFactor(double factor);
    void setEarlyTermination(bool enable);
    QVector<int> decode(const QVector<double>& llr);
    QVector<int> decodeMinSum(const QVector<double>& llr);
    QVector<int> decodeLayered(const QVector<double>& llr);
    int lastIterations() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeComplete(int iterations, bool converged);
private:
    QVector<QVector<int>> m_H; int m_maxIter = 50;
    double m_scaling = 0.75; bool m_earlyStop = true; int m_lastIter = 0;
    int m_m = 0, m_n = 0;
    Stats m_stats; double m_timeSum = 0.0;
};

/**
 * @file TurboCode4.h
 * @brief Turbo码4 — 双二进制+自交织
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class TurboCode4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TurboCode4(QObject* parent = nullptr);

    void setConstraintLength(int K);
    void setMaxIterations(int maxIter);
    void setInterleaver(const QVector<int>& interleaver);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);

    int constraintLength() const { return m_K; }
    int maxIterations() const { return m_maxIterations; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int iterations, bool converged);

private:
    int m_K = 3;
    int m_maxIterations = 8;
    QVector<int> m_interleaver;
    QVector<int> m_generator;

    QVector<double> constituentDecode(const QVector<double>& systematic,
                                       const QVector<double>& parity,
                                       const QVector<double>& prior,
                                       bool terminated) const;
    double sigmaCompute(double extrinsic, double prior) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

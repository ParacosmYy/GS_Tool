/**
 * @file PrunedFFT3.h
 * @brief 剪枝FFT3 — 输入/输出稀疏自适应FFT
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class PrunedFFT3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalTransforms = 0;
        int totalPointsProcessed = 0;
        int totalPrunedNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PrunedFFT3(QObject* parent = nullptr);

    void setSize(int n);
    void setInputMask(const QVector<int>& activeInputs);
    void setOutputMask(const QVector<int>& activeOutputs);
    QVector<double> forward(const QVector<double>& real, const QVector<double>& imag);
    QVector<double> inverse(const QVector<double>& real, const QVector<double>& imag);

    int size() const { return m_n; }
    double efficiency() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, int prunedNodes);

private:
    int m_n = 256;
    int m_logN = 8;
    QVector<int> m_inputMask;
    QVector<int> m_outputMask;
    QVector<double> m_twiddleReal;
    QVector<double> m_twiddleImag;

    void computeTwiddles();
    int reverseBits(int x, int bits) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

/**
 * @file PolyphaseFilterbank3.h
 * @brief 多相滤波器组3 — 临界采样+完整重建
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class PolyphaseFilterbank3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int numBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolyphaseFilterbank3(int numBands = 32, QObject* parent = nullptr);

    void setNumBands(int bands);
    void setFilterLength(int length);
    void setSampleRate(double sampleRate);
    QVector<QVector<double>> analyze(const QVector<double>& input);
    QVector<double> synthesize(const QVector<QVector<double>>& subbands);
    QVector<double> prototypeFilter() const { return m_protoFilter; }

    int numBands() const { return m_numBands; }
    bool isNearPerfect() const { return m_nearPerfect; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int samples, int bands);
    void synthesisCompleted(int samples);

private:
    int m_numBands = 32;
    int m_filterLength = 512;
    double m_sampleRate = 44100.0;
    bool m_nearPerfect = true;
    QVector<double> m_protoFilter;
    QVector<QVector<double>> m_polyMatrix;
    QVector<QVector<double>> m_stateBuffer;

    void designPrototype();
    void buildPolyMatrix();
    void fft(QVector<double>& real, QVector<double>& imag, int n) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};

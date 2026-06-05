/**
 * @file MultibandCompressor.h
 * @brief 多频段压缩器 — 交叉+独立压缩+ makeup
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class MultibandCompressor : public QObject
{
    Q_OBJECT

public:
    struct BandParams {
        double freq = 1000.0;
        double threshold = -20.0;
        double ratio = 4.0;
        double attack = 10.0;
        double release = 100.0;
        double makeup = 0.0;
    };

    struct Stats {
        int totalProcessCalls = 0;
        int totalSamplesProcessed = 0;
        int numBands = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompressor(int numBands = 4, QObject* parent = nullptr);

    void setSampleRate(double sampleRate);
    void setBandParams(int band, const BandParams& params);
    QVector<double> process(const QVector<double>& input);
    QVector<double> bandLevels() const;

    int numBands() const { return m_numBands; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void processingCompleted(int samples, double peakReduction);

private:
    int m_numBands = 4;
    double m_sampleRate = 44100.0;
    QVector<BandParams> m_params;
    QVector<double> m_envelopeState;
    QVector<QVector<double>> m_linkCoeffs;
    QVector<QVector<double>> m_crossoverA;
    QVector<QVector<double>> m_crossoverB;
    QVector<double> m_bandLevels;

    void designCrossovers();
    QVector<QVector<double>> splitBands(const QVector<double>& input);
    double compress(double level, const BandParams& p);

    Stats m_stats;
    double m_timeSum = 0.0;
};

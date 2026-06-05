#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class MultibandCompress2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit MultibandCompress2(QObject* parent = nullptr);
    void setNumBands(int n);
    void setCrossoverFreqs(const QVector<double>& freqs);
    void setBandThreshold(int band, double thresh);
    void setBandRatio(int band, double ratio);
    QVector<double> process(const QVector<double>& input);
    int numBands() const { return m_numBands; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double peakGain);
private:
    int m_numBands = 4;
    QVector<double> m_crossovers; QVector<double> m_thresholds; QVector<double> m_ratios;
    QVector<QVector<double>> splitBands(const QVector<double>& sig);
    QVector<double> compressBand(const QVector<double>& band, double thresh, double ratio);
    Stats m_stats; double m_timeSum = 0.0;
};

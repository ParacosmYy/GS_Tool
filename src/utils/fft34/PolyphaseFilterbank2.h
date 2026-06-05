/**
 * @file PolyphaseFilterbank2.h
 * @brief Polyphase filterbank enhanced - analysis/synthesis/reconstruction
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class PolyphaseFilterbank2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolyphaseFilterbank2(QObject* parent = nullptr);
    void setBands(int bands);
    void setTapsPerBand(int taps);
    void setSampleRate(double rate);
    QVector<QVector<double>> analyze(const QVector<double>& input);
    QVector<double> synthesize(const QVector<QVector<double>>& subbands);
    void setPrototypeFilter(const QVector<double>& coeffs);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void analysisComplete(int bands, int samples);
private:
    int m_bands = 32; int m_tapsPerBand = 8; double m_sampleRate = 44100.0;
    QVector<double> m_protoFilter;
    Stats m_stats; double m_timeSum = 0.0;
};

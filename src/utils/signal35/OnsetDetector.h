/**
 * @file OnsetDetector.h
 * @brief Onset detection - spectral flux/HFC/complex domain/peak picking
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
class OnsetDetector : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalOnsetsFound = 0; int totalFramesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit OnsetDetector(QObject* parent = nullptr);
    void setMethod(int method);
    void setFrameSize(int size);
    void setHopSize(int hop);
    void setSampleRate(double rate);
    void setThreshold(double threshold);
    QList<int> detect(const QVector<double>& audio);
    QVector<QPair<int,double>> detectionFunction(const QVector<double>& audio);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void onsetDetected(int frameIndex, double strength);
    void detectionComplete(int numOnsets);
private:
    int m_method = 0; int m_frameSize = 1024; int m_hopSize = 512;
    double m_sampleRate = 44100.0; double m_threshold = 1.5;
    Stats m_stats; double m_timeSum = 0.0;
};

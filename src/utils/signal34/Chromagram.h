/**
 * @file Chromagram.h
 * @brief Chromagram analysis - chroma features/key estimation/chord detection
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class Chromagram : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Chromagram(QObject* parent = nullptr);
    void setSampleRate(double rate);
    void setFFTSize(int size);
    void setHopSize(int hop);
    void setReferenceFreq(double freq);
    QVector<double> computeFrame(const QVector<double>& frame);
    QVector<QVector<double>> compute(const QVector<double>& audio);
    int estimateKey(const QVector<double>& chroma) const;
    QVector<QPair<double,int>> chordDetect(const QVector<double>& chroma) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void frameComputed(int frameIndex);
private:
    void createChromaMap();
    double m_sampleRate = 44100.0; int m_fftSize = 4096; int m_hopSize = 2048;
    double m_refFreq = 261.63; QVector<QVector<int>> m_chromaMap;
    Stats m_stats; double m_timeSum = 0.0;
};

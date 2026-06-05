#pragma once
#include <QObject>
#include <QVector>
/** @brief Zoom-FFT enhanced - frequency zoom/bandpass/downsample */
class ZoomFFT2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit ZoomFFT2(QObject* parent = nullptr);
    void setSampleRate(double rate); void setCenterFreq(double freq);
    void setBandwidth(double bw); void setOutputBins(int bins);
    QVector<double> transform(const QVector<double>& input);
    double frequencyResolution() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int bins);
private:
    double m_sampleRate = 44100.0; double m_centerFreq = 1000.0;
    double m_bandwidth = 500.0; int m_outputBins = 1024;
    Stats m_stats; double m_timeSum = 0.0;
};

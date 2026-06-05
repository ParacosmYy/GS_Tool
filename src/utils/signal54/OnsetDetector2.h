#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class OnsetDetector2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit OnsetDetector2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setHopSize(int hop);
    void setMethod(const QString& method);
    QVector<double> detect(const QVector<double>& signal);
    QVector<double> onsetFunction() const { return m_onsetEnv; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void onsetDetected(double time, double strength);
private:
    double m_sampleRate = 44100.0; int m_hopSize = 512;
    QString m_method = "spectral_flux";
    QVector<double> m_onsetEnv;
    QVector<double> spectralFlux(const QVector<double>& signal);
    QVector<double> highFrequencyContent(const QVector<double>& signal);
    Stats m_stats; double m_timeSum = 0.0;
};

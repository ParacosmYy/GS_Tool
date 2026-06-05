#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PitchDetector2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDetections = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit PitchDetector2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setMethod(const QString& method);
    double detect(const QVector<double>& frame);
    QVector<QPair<double,double>> detectSequence(const QVector<double>& signal, int frameSize, int hop);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void pitchDetected(double freq, double confidence);
private:
    double m_sampleRate = 44100.0; QString m_method = "yin";
    double detectACF(const QVector<double>& frame) const;
    double detectYIN(const QVector<double>& frame) const;
    Stats m_stats; double m_timeSum = 0.0;
};

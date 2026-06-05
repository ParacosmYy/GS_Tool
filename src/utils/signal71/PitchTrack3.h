#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PitchTrack3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTrackings = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit PitchTrack3(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setMinFreq(double fmin);
    void setMaxFreq(double fmax);
    void setAlgorithm(const QString& algo);
    QVector<double> track(const QVector<double>& signal);
    QVector<double> pitchCurve() const { return m_curve; }
    QVector<double> confidenceCurve() const { return m_conf; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void frameProcessed(int frame, double freq, double conf);
private:
    double m_sampleRate = 44100.0; double m_fmin = 80.0; double m_fmax = 400.0;
    QString m_algo = "yin";
    QVector<double> m_curve; QVector<double> m_conf;
    double detectYin(const QVector<double>& frame);
    double detectACF(const QVector<double>& frame);
    Stats m_stats; double m_timeSum = 0.0;
};

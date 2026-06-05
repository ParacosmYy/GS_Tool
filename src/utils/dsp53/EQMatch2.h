#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class EQMatch2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalMatches = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit EQMatch2(QObject* parent = nullptr);
    void setSampleRate(double sr);
    void setFFTSize(int n);
    void setReference(const QVector<double>& ref);
    QVector<double> match(const QVector<double>& target);
    QVector<double> eqCurve() const { return m_eqCurve; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void matchCompleted(double error);
private:
    double m_sampleRate = 44100.0; int m_fftSize = 2048;
    QVector<double> m_refSpectrum; QVector<double> m_eqCurve;
    bool m_refSet = false;
    void fft(QVector<double>& re, QVector<double>& im) const;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ShortTimeFFT4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit ShortTimeFFT4(QObject* parent = nullptr);
    void setWindowSize(int n);
    void setHopSize(int hop);
    void setWindowType(const QString& type);
    QVector<QVector<double>> forward(const QVector<double>& signal);
    QVector<double> inverse(const QVector<QVector<double>>& stft);
    int windowSize() const { return m_winSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int frames, int bins);
private:
    int m_winSize = 1024; int m_hopSize = 512; QString m_winType = "hann";
    QVector<double> generateWindow(int n, const QString& type);
    Stats m_stats; double m_timeSum = 0.0;
};

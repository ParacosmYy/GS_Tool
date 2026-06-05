#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PhaseVocoder3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit PhaseVocoder3(QObject* parent = nullptr);
    void setWindowSize(int n);
    void setHopSize(int hop);
    void setStretchFactor(double factor);
    QVector<double> process(const QVector<double>& input);
    double stretchFactor() const { return m_factor; }
    int outputLength(int inputLen) const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int inLen, int outLen);
private:
    int m_winSize = 2048; int m_hopSize = 512; double m_factor = 1.5;
    QVector<double> generateWindow(int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};

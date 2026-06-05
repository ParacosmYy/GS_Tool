#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class GaborTransform2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit GaborTransform2(QObject* parent = nullptr);
    void setWindowSize(int n);
    void setHopSize(int hop);
    void setNumFrequencyBins(int bins);
    QVector<QVector<double>> forward(const QVector<double>& signal);
    QVector<double> inverse(const QVector<QVector<double>>& transform);
    int windowSize() const { return m_winSize; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int frames, int bins);
private:
    int m_winSize = 256; int m_hopSize = 128; int m_numBins = 256;
    QVector<double> m_gabor;
    void designGaborAtom();
    Stats m_stats; double m_timeSum = 0.0;
};

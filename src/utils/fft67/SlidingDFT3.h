#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SlidingDFT3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalUpdates = 0; int totalBins = 0; double avgProcessingTimeMs = 0.0; };
    explicit SlidingDFT3(QObject* parent = nullptr);
    void setSize(int n);
    void setDamping(double damp);
    void pushSample(double sample);
    QVector<double> spectrum() const;
    double bin(int k) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void updated(int bin);
private:
    int m_n = 256; double m_damp = 0.999;
    QVector<double> m_X; QVector<double> m_Xprev;
    int m_pos = 0;
    QVector<double> m_twiddleRe; QVector<double> m_twiddleIm;
    void initTwiddleFactors();
    Stats m_stats; double m_timeSum = 0.0;
};

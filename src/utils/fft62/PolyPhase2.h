#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PolyPhase2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalChannels = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolyPhase2(QObject* parent = nullptr);
    void setNumChannels(int n);
    void setFilterLength(int len);
    void setOversampling(int os);
    QVector<QVector<double>> analyze(const QVector<double>& input);
    QVector<double> synthesize(const QVector<QVector<double>>& bands);
    int numChannels() const { return m_numChan; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int channels, int samples);
private:
    int m_numChan = 8; int m_filterLen = 64; int m_oversamp = 1;
    QVector<double> m_protoFilter;
    void designPrototype();
    Stats m_stats; double m_timeSum = 0.0;
};

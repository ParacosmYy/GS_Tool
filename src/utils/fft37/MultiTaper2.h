#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class MultiTaper2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEstimates = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit MultiTaper2(QObject* parent = nullptr);
    void setTapers(int nw, int k); void setSampleRate(double rate);
    QVector<double> estimate(const QVector<double>& input);
    QVector<QVector<double>> taperSequences() const;
    QVector<double> taperEigenvalues() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void estimateComplete(int bins);
private:
    int m_nw = 4, m_k = 7; double m_sampleRate = 44100.0;
    QVector<QVector<double>> m_tapers; QVector<double> m_eigenvalues;
    Stats m_stats; double m_timeSum = 0.0;
};

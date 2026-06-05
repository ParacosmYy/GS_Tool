#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BlindSourceSep2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSeparations = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit BlindSourceSep2(QObject* parent = nullptr);
    void setNumSources(int n);
    void setLearningRate(double lr);
    QVector<QVector<double>> separate(const QVector<QVector<double>>& mixtures);
    QVector<QVector<double>> mixingMatrix() const { return m_W; }
    int numSources() const { return m_nSources; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void separationCompleted(int sources, double convergence);
private:
    int m_nSources = 2; double m_lr = 0.01;
    QVector<QVector<double>> m_W;
    double g(double x) const;
    double gPrime(double x) const;
    Stats m_stats; double m_timeSum = 0.0;
};

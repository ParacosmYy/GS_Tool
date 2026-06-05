#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BlindSourceSep3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSeparations = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit BlindSourceSep3(QObject* parent = nullptr);
    void setNumSources(int n);
    void setLearningRate(double lr);
    void setMaxIterations(int iter);
    void setConvergenceTol(double tol);
    QVector<QVector<double>> separate(const QVector<QVector<double>>& mixtures);
    QVector<QVector<double>> unmixingMatrix() const { return m_W; }
    double convergence() const { return m_convergence; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void separationCompleted(int sources, double convergence);
private:
    int m_nSources = 2; double m_lr = 0.01; int m_maxIter = 1000; double m_tol = 1e-6;
    QVector<QVector<double>> m_W; double m_convergence = 0.0;
    double g(double x) const;
    double gPrime(double x) const;
    Stats m_stats; double m_timeSum = 0.0;
};

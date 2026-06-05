#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class Multitaper3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEstimates = 0; int totalTapers = 0; double avgProcessingTimeMs = 0.0; };
    explicit Multitaper3(QObject* parent = nullptr);
    void setSize(int n);
    void setNumTapers(int nw);
    void setBandwidth(double nw);
    QVector<double> estimate(const QVector<double>& signal);
    QVector<QVector<double>> tapers() const { return m_tapers; }
    QVector<double> eigenvalues() const { return m_eigenvalues; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void estimateCompleted(int tapers, int size);
private:
    int m_n = 1024; int m_numTapers = 5; double m_nw = 4.0;
    QVector<QVector<double>> m_tapers; QVector<double> m_eigenvalues;
    void computeDPSS(int n, int k, double nw);
    Stats m_stats; double m_timeSum = 0.0;
};

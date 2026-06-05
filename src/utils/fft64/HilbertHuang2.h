#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HilbertHuang2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; int totalIMFs = 0; double avgProcessingTimeMs = 0.0; };
    explicit HilbertHuang2(QObject* parent = nullptr);
    void setMaxIMFs(int n);
    void setMaxSiftIterations(int iter);
    void setSDThreshold(double sd);
    QVector<QVector<double>> decompose(const QVector<double>& signal);
    QVector<QVector<double>> hilbertSpectrum(const QVector<QVector<double>>& imfs);
    int numIMFs() const { return m_numIMFs; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decompositionCompleted(int imfs, int totalSifts);
private:
    int m_maxIMFs = 10; int m_maxSiftIter = 1000; double m_sdThresh = 0.05;
    int m_numIMFs = 0;
    QVector<double> findSift(const QVector<double>& sig);
    QVector<double> envelopeMean(const QVector<double>& sig);
    Stats m_stats; double m_timeSum = 0.0;
};

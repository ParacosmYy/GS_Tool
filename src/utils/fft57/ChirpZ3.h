#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ChirpZ3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit ChirpZ3(QObject* parent = nullptr);
    void setSize(int n);
    void setFreqRange(double f0, double f1);
    void setNumOutputPoints(int m);
    QVector<double> forward(const QVector<double>& re, const QVector<double>& im);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n, int m);
private:
    int m_n = 256; int m_m = 256; double m_f0 = 0.0; double m_f1 = 1.0;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class WignerVille2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDistributions = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit WignerVille2(QObject* parent = nullptr);
    void setSignalLength(int n);
    void setWindowType(const QString& type);
    QVector<QVector<double>> compute(const QVector<double>& signal);
    int signalLength() const { return m_n; }
    double maxFrequency() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void distributionComputed(int n, int timeBins);
private:
    int m_n = 256; QString m_winType = "hann";
    QVector<double> generateWindow(int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};

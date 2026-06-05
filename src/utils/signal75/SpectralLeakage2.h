#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpectralLeakage2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalComputations = 0; int totalWindows = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpectralLeakage2(QObject* parent = nullptr);
    void setWindowSize(int n);
    void setWindowType(const QString& type);
    double computeENBW();
    double computeSCALOSS();
    QVector<double> generateWindow(int n, const QString& type);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(double enbw, double scaloss);
private:
    int m_winSize = 1024; QString m_winType = "hann";
    Stats m_stats; double m_timeSum = 0.0;
};

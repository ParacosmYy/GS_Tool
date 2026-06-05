#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ZoomFFT3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit ZoomFFT3(QObject* parent = nullptr);
    void setInputSize(int n);
    void setZoomRange(double fCenter, double fSpan);
    void setOutputSize(int m);
    QVector<double> forward(const QVector<double>& re, const QVector<double>& im);
    double frequencyResolution() const;
    int outputSize() const { return m_m; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n, int m);
private:
    int m_n = 1024; int m_m = 256; double m_fCenter = 0.0; double m_fSpan = 0.1;
    Stats m_stats; double m_timeSum = 0.0;
};

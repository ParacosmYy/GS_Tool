#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class StereoWidth3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessings = 0; int totalSamples = 0; double avgProcessingTimeMs = 0.0; };
    explicit StereoWidth3(QObject* parent = nullptr);
    void setWidth(double w);
    void setCenterLevel(double level);
    void setLink(bool link);
    QVector<QVector<double>> process(const QVector<QVector<double>>& input);
    double width() const { return m_width; }
    double correlation() const { return m_correlation; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void processingCompleted(int samples, double width);
private:
    double m_width = 1.0; double m_centerLevel = 1.0; bool m_link = true;
    double m_correlation = 0.0;
    Stats m_stats; double m_timeSum = 0.0;
};

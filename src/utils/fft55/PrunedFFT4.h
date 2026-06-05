#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PrunedFFT4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit PrunedFFT4(QObject* parent = nullptr);
    void setSize(int n);
    void setInputMask(const QVector<int>& active);
    void setOutputMask(const QVector<int>& active);
    QVector<double> forward(const QVector<double>& re, const QVector<double>& im);
    double efficiency() const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    QVector<int> m_inMask; QVector<int> m_outMask;
    Stats m_stats; double m_timeSum = 0.0;
};

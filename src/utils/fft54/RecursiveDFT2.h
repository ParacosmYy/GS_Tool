#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class RecursiveDFT2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit RecursiveDFT2(QObject* parent = nullptr);
    void setSize(int n);
    QPair<QVector<double>,QVector<double>> forward(const QVector<double>& re, const QVector<double>& im);
    QPair<QVector<double>,QVector<double>> inverse(const QVector<double>& re, const QVector<double>& im);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    void ditfft(const QVector<double>& re, const QVector<double>& im,
                QVector<double>& outRe, QVector<double>& outIm, int n, int stride);
    Stats m_stats; double m_timeSum = 0.0;
};

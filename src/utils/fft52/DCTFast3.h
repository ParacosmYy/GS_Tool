#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DCTFast3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit DCTFast3(QObject* parent = nullptr);
    void setSize(int n);
    QVector<double> forward(const QVector<double>& input) const;
    QVector<double> inverse(const QVector<double>& coeffs) const;
    QVector<QVector<double>> forward2D(const QVector<QVector<double>>& input) const;
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    void fft(QVector<double>& re, QVector<double>& im, int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};

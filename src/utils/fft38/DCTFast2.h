#pragma once
#include <QObject>
#include <QVector>
class DCTFast2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit DCTFast2(QObject* parent = nullptr);
    void setSize(int n);
    QVector<double> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<double>& coeffs);
    QVector<QVector<double>> forward2D(const QVector<QVector<double>>& matrix);
    QVector<QVector<double>> inverse2D(const QVector<QVector<double>>& matrix);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int size);
private:
    int m_n = 256;
    Stats m_stats; double m_timeSum = 0.0;
};

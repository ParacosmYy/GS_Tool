#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class DCTFast4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit DCTFast4(QObject* parent = nullptr);
    void setSize(int n);
    QVector<double> forward(const QVector<double>& data);
    QVector<double> inverse(const QVector<double>& coeffs);
    QVector<double> forward2D(const QVector<QVector<double>>& data);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    QVector<double> dctII(const QVector<double>& x);
    QVector<double> dctIII(const QVector<double>& x);
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class FastHartley3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit FastHartley3(QObject* parent = nullptr);
    void setSize(int n);
    QVector<double> forward(const QVector<double>& data);
    QVector<double> inverse(const QVector<double>& data);
    QVector<double> convolve(const QVector<double>& a, const QVector<double>& b);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    void fht(QVector<double>& data);
    Stats m_stats; double m_timeSum = 0.0;
};

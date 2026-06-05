#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class WalshHadamard3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit WalshHadamard3(QObject* parent = nullptr);
    void setSize(int n);
    QVector<double> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<double>& transformed);
    QVector<double> sequencySpectrum(const QVector<double>& signal);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256;
    void fwt(QVector<double>& data, int n) const;
    Stats m_stats; double m_timeSum = 0.0;
};

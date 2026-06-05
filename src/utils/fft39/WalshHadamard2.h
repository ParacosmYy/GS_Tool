#pragma once
#include <QObject>
#include <QVector>
class WalshHadamard2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalSamplesProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit WalshHadamard2(QObject* parent = nullptr);
    QVector<double> forward(const QVector<double>& input);
    QVector<double> inverse(const QVector<double>& input);
    QVector<double> sequencyOrder(const QVector<double>& input) const;
    QVector<double> walshSpectrum(const QVector<double>& input);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformComplete(int size);
private:
    int nextPowerOfTwo(int n) const;
    QVector<double> padToLength(const QVector<double>& input, int len) const;
    int bitReverse(int value, int bits) const;
    void updateTimeStats(double elapsed) const;
    mutable Stats m_stats; mutable double m_timeSum = 0.0;
};

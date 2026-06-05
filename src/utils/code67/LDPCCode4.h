#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class LDPCCode4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit LDPCCode4(QObject* parent = nullptr);
    void setBlockLength(int n);
    void setCodeRate(double rate);
    void setDecodingAlgorithm(const QString& algo);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& llr);
    int blockLength() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int iterations, bool converged);
private:
    int m_n = 648; double m_rate = 0.5; QString m_algo = "minsum";
    QVector<QVector<int>> m_H; QVector<QVector<int>> m_Hcol;
    void loadParityCheckMatrix();
    QVector<double> minSumDecode(const QVector<double>& llr);
    QVector<double> sumProductDecode(const QVector<double>& llr);
    Stats m_stats; double m_timeSum = 0.0;
};

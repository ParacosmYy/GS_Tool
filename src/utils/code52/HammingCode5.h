#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HammingCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalErrors = 0; double avgProcessingTimeMs = 0.0; };
    explicit HammingCode5(QObject* parent = nullptr);
    void setDataBits(int bits);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int codeLength() const { return m_n; }
    int dataBits() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int errors);
private:
    int m_k = 11; int m_r = 4; int m_n = 15;
    QVector<QVector<int>> m_parity;
    void buildParity();
    Stats m_stats; double m_timeSum = 0.0;
};

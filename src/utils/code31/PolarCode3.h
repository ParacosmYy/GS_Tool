/**
 * @file PolarCode3.h
 * @brief Polar code enhanced - CA-SCL decoder
 */
#pragma once
#include <QObject>
#include <QVector>
class PolarCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolarCode3(QObject* parent = nullptr);
    void configure(int n, int k, int listSize = 8);
    void setCRC(int crcBits);
    QVector<int> encode(const QVector<int>& message) const;
    QVector<int> decode(const QVector<double>& llr);
    void setFrozenSet(const QVector<int>& frozen);
    int n() const; int k() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeComplete(bool crcPassed);
private:
    void scDecodeRecursive(QVector<double>& llr, QVector<int>& uHat, int start, int len) const;
    double signProd(double a, double b) const;
    bool checkCRC(const QVector<int>& bits) const;
    void updateLLR(QVector<double>& llr, const QVector<int>& bits, int idx) const;
    void generateFrozenSet();
    double bhattacharyya(int index) const;
    int m_n = 0, m_k = 0, m_listSize = 8, m_crcBits = 16;
    QVector<int> m_frozen;
    Stats m_stats; double m_timeSum = 0.0;
};

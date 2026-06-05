#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PolarCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolarCode5(QObject* parent = nullptr);
    void setParameters(int n, int k, int listSize = 8);
    QVector<int> encode(const QVector<int>& info);
    QVector<int> decode(const QVector<double>& llr);
    int blockLength() const { return m_n; }
    int infoLength() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int n, int k);
    void decodeCompleted(bool crcPass);
private:
    int m_n = 128; int m_k = 64; int m_listSize = 8;
    QVector<int> m_frozen; QVector<int> m_crcPoly;
    void generateFrozen(); quint32 crc(const QVector<int>& bits) const;
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class LdpcEncoder2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalBits = 0; double avgProcessingTimeMs = 0.0; };
    explicit LdpcEncoder2(QObject* parent = nullptr);
    void setParityMatrix(const QVector<QVector<int>>& H);
    void setSystematic(bool systematic);
    QVector<int> encode(const QVector<int>& data);
    int codeLength() const { return m_n; }
    int dataLength() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int dataBits, int codeBits);
private:
    int m_n = 0; int m_k = 0; bool m_systematic = true;
    QVector<QVector<int>> m_H;
    QVector<QVector<int>> m_G;
    void generateSystematic();
    Stats m_stats; double m_timeSum = 0.0;
};

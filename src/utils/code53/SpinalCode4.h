#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpinalCode4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpinalCode4(QObject* parent = nullptr);
    void setSeed(quint32 seed);
    QVector<int> encode(const QVector<int>& msg, int passes = 4);
    QVector<int> decode(const QVector<int>& rx, int beamWidth = 16, int msgLen = -1);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int len, bool ok);
private:
    quint32 m_seed = 0x12345678; int m_k = 4;
    quint32 spineFn(quint32 sv, quint32 bits) const;
    int rngSym(quint32 sv, int idx) const;
    Stats m_stats; double m_timeSum = 0.0;
};

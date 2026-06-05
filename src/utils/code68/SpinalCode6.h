#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class SpinalCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpinalCode6(QObject* parent = nullptr);
    void setSpineLength(int n);
    void setBeamWidth(int w);
    void setNumPasses(int p);
    void setK(int k);
    QVector<double> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& symbols);
    int spineLength() const { return m_spineLen; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int passes, bool converged);
private:
    int m_spineLen = 64; int m_beamWidth = 16; int m_numPasses = 4; int m_k = 4;
    quint32 hashFunc(quint32 spine, const QVector<int>& bits);
    QVector<double> rngFunc(quint32 seed, int count);
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ConvolutionalCode4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit ConvolutionalCode4(QObject* parent = nullptr);
    void setGenerators(const QVector<int>& gens);
    void setConstraintLength(int k);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);
    int freeDistance() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int inLen, int outLen);
private:
    int m_constraint = 7; QVector<int> m_gens;
    Stats m_stats; double m_timeSum = 0.0;
};

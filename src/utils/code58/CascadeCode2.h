#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class CascadeCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit CascadeCode2(QObject* parent = nullptr);
    void setOuterCode(int n, int k);
    void setInnerCode(int n, int k);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& softBits);
    int outerCodeN() const { return m_outerN; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int outerErrors, int innerErrors);
private:
    int m_outerN = 31; int m_outerK = 25; int m_innerN = 15; int m_innerK = 11;
    Stats m_stats; double m_timeSum = 0.0;
};

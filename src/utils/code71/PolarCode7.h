#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class PolarCode7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit PolarCode7(QObject* parent = nullptr);
    void setBlockLength(int n);
    void setInfoLength(int k);
    void setDecodingMethod(const QString& method);
    QVector<int> encode(const QVector<int>& bits);
    QVector<int> decode(const QVector<double>& llr);
    int blockLength() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int errors, bool success);
private:
    int m_n = 128; int m_k = 64; QString m_method = "sc";
    QVector<int> m_frozenSet;
    void designFrozenSet();
    QVector<int> scDecode(const QVector<double>& llr);
    QVector<int> sclDecode(const QVector<double>& llr, int listSize);
    Stats m_stats; double m_timeSum = 0.0;
};

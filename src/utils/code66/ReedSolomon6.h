#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ReedSolomon6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit ReedSolomon6(QObject* parent = nullptr);
    void setFieldOrder(int m);
    void setNumParity(int npar);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int n() const { return m_n; }
    int k() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int errors, bool success);
private:
    int m_m = 8; int m_n = 255; int m_k = 223; int m_npar = 32;
    QVector<int> m_expTable; QVector<int> m_logTable;
    void initGField();
    int gfMul(int a, int b) const;
    int gfInv(int a) const;
    QVector<int> calcSyndromes(const QVector<int>& data);
    QVector<int> berlekampMassey(const QVector<int>& synd);
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class BchCode5 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit BchCode5(QObject* parent = nullptr);
    void setFieldOrder(int m);
    void setDesiredDistance(int d);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int n() const { return m_n; }
    int k() const { return m_k; }
    int t() const { return m_t; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int errors, bool corrected);
private:
    int m_m = 5; int m_n = 31; int m_k = 16; int m_t = 3;
    QVector<int> m_gPoly;
    void computeGeneratorPoly();
    QVector<int> berlekampMassey(const QVector<int>& syndrome);
    QVector<int> chienSearch(const QVector<int>& sigma);
    Stats m_stats; double m_timeSum = 0.0;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class HammingCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; double avgProcessingTimeMs = 0.0; };
    explicit HammingCode6(QObject* parent = nullptr);
    void setRedundancy(int r);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int n() const { return m_n; }
    int k() const { return m_k; }
    int minDistance() const { return 3; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeCompleted(int syndrome, bool corrected);
private:
    int m_r = 4; int m_n = 15; int m_k = 11;
    QVector<QVector<int>> m_H; QVector<QVector<int>> m_G;
    void buildParityCheckMatrix();
    void buildGeneratorMatrix();
    Stats m_stats; double m_timeSum = 0.0;
};

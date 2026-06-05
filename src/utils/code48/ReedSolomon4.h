#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ReedSolomon4 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalErrors = 0; double avgProcessingTimeMs = 0.0; };
    explicit ReedSolomon4(QObject* parent = nullptr);
    void setParameters(int n, int k);
    QVector<int> encode(const QVector<int>& message);
    QVector<int> decode(const QVector<int>& received);
    bool isCodeword(const QVector<int>& data) const;
    int codeLength() const { return m_n; }
    int dataLength() const { return m_k; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeCompleted(int n, int k);
    void decodeCompleted(int errors);
private:
    int m_n = 255; int m_k = 223; int m_t = 16;
    QVector<int> m_genPoly; QVector<int> m_alpha; QVector<int> m_index;
    void generateTables(); int gfMul(int a, int b) const; int gfInv(int a) const;
    QVector<int> calcSyndromes(const QVector<int>& r) const;
    Stats m_stats; double m_timeSum = 0.0;
};

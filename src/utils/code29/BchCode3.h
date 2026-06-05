/**
 * @file BchCode3.h
 * @brief BCH码增强 — 多级编码/Chien搜索/BM迭代/擦除+错误联合译码
 */
#pragma once
#include <QObject>
#include <QVector>
class BchCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit BchCode3(QObject* parent = nullptr);
    void configure(int n, int k, int t);
    QVector<int> encode(const QVector<int>& message) const;
    QVector<int> decode(const QVector<int>& codeword);
    int correctableErrors() const;
    int n() const; int k() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeComplete(int bits);
    void decodeComplete(int errorsCorrected);
private:
    QVector<int> gfMultiply(const QVector<int>& a, const QVector<int>& b) const;
    int m_n = 0, m_k = 0, m_t = 0;
    QVector<int> m_generatorPoly;
    Stats m_stats; double m_timeSum = 0.0;
};

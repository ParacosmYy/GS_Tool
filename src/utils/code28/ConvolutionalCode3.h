/**
 * @file ConvolutionalCode3.h
 * @brief 卷积码增强 — 软判决Viterbi/BCJR前向后向/递归系统码
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
class ConvolutionalCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalBitsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };
    explicit ConvolutionalCode3(QObject* parent = nullptr);
    void setGeneratorPolynomials(const QVector<int>& polys, int constraintLength);
    void setRecursivePolynomial(int feedbackPoly);
    QVector<int> encode(const QVector<int>& bits) const;
    QVector<int> decodeViterbi(const QVector<double>& softBits);
    QVector<double> decodeBCJR(const QVector<double>& llr);
    void setTermination(bool terminate);
    int constraintLength() const;
    int codeRate() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void encodeComplete(int numBits);
    void decodeComplete(int numBits, double ber);
private:
    QVector<int> m_polys;
    int m_constraint = 7;
    int m_feedbackPoly = 0;
    bool m_recursive = false;
    bool m_terminated = true;
    int m_numStates = 64;
    Stats m_stats;
    double m_timeSum = 0.0;
};

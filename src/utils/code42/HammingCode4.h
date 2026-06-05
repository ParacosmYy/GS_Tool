/**
 * @file HammingCode4.h
 * @brief 汉明码4 — 扩展汉明(SEC-DED)+多位纠错
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class HammingCode4 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalEncodes = 0;
        int totalDecodes = 0;
        int totalErrorsCorrected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HammingCode4(QObject* parent = nullptr);

    void setParameters(int dataBits, bool extendedParity = true);
    QVector<int> encode(const QVector<int>& data);
    QVector<int> decode(const QVector<int>& received);
    int detectErrors(const QVector<int>& received) const;
    QVector<int> syndrome(const QVector<int>& received) const;

    int codeLength() const { return m_n; }
    int dataBits() const { return m_k; }
    int parityBits() const { return m_r; }
    bool isExtended() const { return m_extended; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int dataBits, int codeBits);
    void decodeCompleted(int errorsDetected, int errorsCorrected);

private:
    int m_k = 4;
    int m_r = 3;
    int m_n = 7;
    bool m_extended = true;
    QVector<QVector<int>> m_parityMatrix;

    void buildParityMatrix();

    Stats m_stats;
    double m_timeSum = 0.0;
};

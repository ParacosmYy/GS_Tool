#pragma once
#include <QObject>
#include <QVector>
/** @brief Hamming code enhanced - SEC-DED/extended */
class HammingCode3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalEncodes = 0; int totalDecodes = 0; int totalBitsProcessed = 0; double avgProcessingTimeMs = 0.0; };
    explicit HammingCode3(QObject* parent = nullptr);
    void setParameters(int m);
    QVector<int> encode(const QVector<int>& data) const;
    QVector<int> decode(const QVector<int>& codeword);
    int detectErrors(const QVector<int>& codeword) const;
    void setExtended(bool ext);
    int dataBits() const; int totalBits() const;
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void decodeComplete(int errorsCorrected);
private:
    int m_m = 3; bool m_extended = false;
    Stats m_stats; double m_timeSum = 0.0;
};

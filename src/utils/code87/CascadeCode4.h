#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 级联码编码器/解码器
 *
 * 内外码级联的纠错编码方案，组合多种编码优势。
 */
class CascadeCode4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit CascadeCode4(QObject* parent = nullptr);

    /** @brief 级联编码(外码+内码) */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 级联解码(内码+外码) */
    QVector<int> decode(const QVector<double>& softBits);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, int outerErrors, int innerErrors);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_outerRedundancy = 0;
    int m_innerRedundancy = 0;
};

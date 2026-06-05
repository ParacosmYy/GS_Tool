#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Spinal码编码器/解码器
 *
 * 基于散列函数的率兼容编码，适用于短包通信场景。
 */
class SpinalCode8 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpinalCode8(QObject* parent = nullptr);

    /** @brief 编码数据 */
    QVector<double> encode(const QVector<int>& data, int spineValueBits = 8);

    /** @brief 解码(基于堆栈解码器) */
    QVector<int> decode(const QVector<double>& received, int spineValueBits = 8, int depth = 10);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, double confidence);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_spineBits = 8;
};

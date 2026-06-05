#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Turbo码信道编码器/解码器
 *
 * 并行级联卷积码实现，支持迭代解码与软判决输出。
 */
class TurboCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TurboCode6(QObject* parent = nullptr);

    /** @brief 编码数据块 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 解码接收到的软比特序列 */
    QVector<int> decode(const QVector<double>& softBits, int iterations = 6);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, double ber);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_iterationCount = 6;
};

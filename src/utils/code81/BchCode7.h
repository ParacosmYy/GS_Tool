#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief BCH纠错码编码器/解码器
 *
 * Bose-Chaudhuri-Hocquenghem码实现，支持多比特纠错。
 */
class BchCode7 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BchCode7(QObject* parent = nullptr);

    /** @brief 编码数据块 */
    QVector<int> encode(const QVector<int>& data, int errorCapability = 2);

    /** @brief 解码并纠错 */
    QVector<int> decode(const QVector<int>& codeword, int errorCapability = 2);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, int errorsCorrected);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_errorCapability = 2;
};

#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Polar码编码器/解码器
 *
 * 基于信道极化的容量达到码，支持SC和SCL解码。
 */
class PolarCode9 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalBlocksEncoded = 0;
        int totalBlocksDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolarCode9(QObject* parent = nullptr);

    /** @brief 编码数据 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief SC(连续消除)解码 */
    QVector<int> decodeSC(const QVector<double>& llr);

    /** @brief SCL(列表)解码 */
    QVector<int> decodeSCL(const QVector<double>& llr, int listSize = 8);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int blockIndex, int listSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_codeLength = 0;
};

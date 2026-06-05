#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Gray码生成器与转换器
 *
 * 生成Gray码序列及二进制/Gray码互转。
 */
class GrayCode6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSequencesGenerated = 0;
        int totalConversions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GrayCode6(QObject* parent = nullptr);

    /** @brief 生成n位Gray码序列 */
    QVector<int> generate(int bits);

    /** @brief 二进制转Gray码 */
    int binaryToGray(int binary) const;

    /** @brief Gray码转二进制 */
    int grayToBinary(int gray) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sequenceGenerated(int bits, int sequenceLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};

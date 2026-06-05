#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 卷积码编解码器
 *
 * 实现卷积码的编码与Viterbi译码，支持可配置的约束长度
 * 和生成多项式，用于通信系统的信道编码。
 */
class ConvCode6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalEncoded = 0;        ///< 已编码比特数
        int totalDecoded = 0;        ///< 已解码比特数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ConvCode6(QObject* parent = nullptr);

    /** @brief 设置约束长度 */
    void setConstraintLen(int length);
    /** @brief 设置生成多项式(八进制表示) */
    void setGenerator(const QVector<int>& gen);
    /** @brief 卷积编码 */
    QVector<int> encode(const QVector<int>& bits);
    /** @brief Viterbi译码(软判决) */
    QVector<int> decode(const QVector<double>& received);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成，返回纠错比特数 */
    void codingCompleted(int correctedBits);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_constraintLen = 7;
    QVector<int> m_generator = {171, 133};
};

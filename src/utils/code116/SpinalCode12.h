#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SpinalCode12 - 脊柱码第12代实现
 *
 * 提供Spinal码的编码与解码功能，支持哈希函数编码、
 * 连续解码及气泡解码器的高效实现。
 */
class SpinalCode12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit SpinalCode12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief Spinal码编码
     * @param message 原始消息比特序列
     * @param k 每步输入比特数
     * @param numPasses 编码通过次数
     * @return 编码后的符号序列
     */
    QVector<double> encode(const QVector<int>& message, int k, int numPasses);

    /**
     * @brief 连续解码器（最优但复杂度高）
     * @param received 接收符号序列
     * @param k 每步输入比特数
     * @param messageLength 消息长度
     * @return 解码后的消息比特序列
     */
    QVector<int> decodeSequential(const QVector<double>& received, int k, int messageLength);

    /**
     * @brief 气泡解码器（近似但速度快）
     * @param received 接收符号序列
     * @param k 每步输入比特数
     * @param messageLength 消息长度
     * @param bubbleWidth 保留的候选路径数
     * @return 解码后的消息比特序列
     */
    QVector<int> decodeBubble(const QVector<double>& received, int k,
                              int messageLength, int bubbleWidth = 16);

signals:
    void decodeCompleted(int pathCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};

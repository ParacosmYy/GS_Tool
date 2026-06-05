#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Turbo码(Turbo Code)编解码器实现
 *
 * 基于并行级联卷积码(PCCC)的迭代解码结构，采用BCJR算法和交织器
 * 实现接近Shannon极限的纠错性能，适用于深空通信和3G/4G移动通信。
 */
class TurboCode9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit TurboCode9(QObject* parent = nullptr);

    /** @brief 设置交织器长度，影响编码增益和延迟 */
    void setInterleaverSize(int size);

    /** @brief 设置迭代解码次数，越多纠错能力越强但延迟越大 */
    void setIterations(int iterations);

    /** @brief 对输入比特流执行Turbo编码，输出编码符号 */
    QVector<int> encode(const QVector<int>& bits);

    /** @brief 对接收软信息执行迭代Turbo解码 */
    QVector<int> decode(const QVector<double>& softBits, int messageLength);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成信号，返回迭代次数和估计误码率 */
    void decodingCompleted(int iterationsUsed, double estimatedBER);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_interleaverSize = 1024;
    int m_iterations = 8;
};

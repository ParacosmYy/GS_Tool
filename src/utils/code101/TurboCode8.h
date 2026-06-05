#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Turbo码编解码器
 *
 * 基于并行级联卷积码(PCCC)的迭代解码实现，
 * 采用MAP/BCJR算法在两个分量解码器间传递软信息。
 */
class TurboCode8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalCoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit TurboCode8(QObject* parent = nullptr);

    /** @brief 设置迭代解码次数 */
    void setIterations(int iterations);

    /** @brief 编码比特序列 */
    QVector<int> encode(const QVector<int>& data);

    /** @brief 解码软比特序列 */
    QVector<int> decode(const QVector<double>& softBits);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编解码完成信号 */
    void codingCompleted(int bitCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_iterations = 8;
};

#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Gray码编解码器
 *
 * 实现二进制与Gray码之间的相互转换，
 * 相邻Gray码仅有一位不同，适用于减少状态切换噪声。
 */
class GrayCode8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalCoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit GrayCode8(QObject* parent = nullptr);

    /** @brief 设置位长度 */
    void setBitLength(int bits);

    /** @brief 将整数编码为Gray码 */
    int encode(int value);

    /** @brief 将Gray码解码为整数 */
    int decode(int grayValue);

    /** @brief 生成所有Gray码序列 */
    QVector<int> generateAll();

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 序列生成完成信号 */
    void generated(int codeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bitLength = 8;
};

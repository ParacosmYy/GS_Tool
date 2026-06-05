#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Gray码编解码器
 *
 * 实现二进制与Gray码之间的相互转换,相邻编码仅有一位差异,
 * 适用于旋转编码器、卡诺图化简与错误最小化数字传输。
 */
class GrayCode7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalGenerated = 0; double avgProcessingTimeMs = 0.0; };

    explicit GrayCode7(QObject* parent = nullptr);

    /** @brief 设置编码位长度 */
    void setBitLength(int bits);

    /** @brief 将整数编码为Gray码 */
    void encode(int value);

    /** @brief 将Gray码解码为整数 */
    void decode(int grayValue);

    /** @brief 生成指定位长的全部Gray码序列 */
    void generateAll();

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 序列生成完成信号 */
    void generated(int codeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bitLength = 8;
};

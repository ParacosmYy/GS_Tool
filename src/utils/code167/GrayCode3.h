/**
 * @file GrayCode3.h
 * @brief 格雷码编解码(二进制-格雷转换与排名/逆排名) — Gray Code Encoder/Decoder with Binary-Gray Conversion and Ranking/Unranking
 *
 * 功能: 实现n位格雷码的编码、解码、排名(rank)与逆排名(unrank)，
 *       支持二进制↔格雷码互转、生成完整格雷码序列、汉明距离计算。
 *
 * 协作: NumberTheoreticTransform(数论变换) / ReedSolomon(纠错码) / HuffmanCoding(压缩编码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 格雷码编解码器
 */
class GrayCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;        ///< 累计编码次数
        quint64 totalDecodes = 0;        ///< 累计解码次数
        double avgProcessingTimeUs = 0.0;///< 平均耗时(μs)
        int lastBitWidth = 0;            ///< 最近位宽
    };

    explicit GrayCode3(QObject *parent = nullptr);
    ~GrayCode3() override;

    /** @brief 设置位宽(1~31) */
    void setBitWidth(int bits);

    /**
     * @brief 二进制转格雷码
     * @param binary 二进制值
     * @return 格雷码值
     */
    quint32 encode(quint32 binary) const;

    /**
     * @brief 格雷码转二进制
     * @param gray 格雷码值
     * @return 二进制值
     */
    quint32 decode(quint32 gray) const;

    /**
     * @brief 格雷码排名(格雷码→序号)
     * @param gray 格雷码值
     * @return 在序列中的排名(0-based)
     */
    quint32 rank(quint32 gray) const;

    /**
     * @brief 格雷码逆排名(序号→格雷码)
     * @param r 排名
     * @return 对应格雷码值
     */
    quint32 unrank(quint32 r) const;

    /** @brief 生成完整格雷码序列 */
    QVector<quint32> generateSequence() const;

    /** @brief 计算两个格雷码的汉明距离 */
    int hammingDistance(quint32 a, quint32 b) const;

    /** @brief 获取当前位宽 */
    int bitWidth() const { return m_bits; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param value 结果值 */
    void encodeCompleted(quint32 value);
    /** @brief 序列生成完成 @param size 序列长度 */
    void sequenceGenerated(int size);

private:
    /** @brief 计算汉明权重 */
    static int popcount(quint32 x);

    int m_bits = 4; ///< 默认4位格雷码

    Stats m_stats;
    double m_timeSum = 0.0;
};

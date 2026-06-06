/**
 * @file LempelZivWelch3.h
 * @brief LZW编解码(变宽编码+字典提前重置) — LZW Encoder/Decoder with Variable-Width Codes and Early Dictionary Reset
 *
 * 功能: 实现LZW压缩算法，支持变宽编码(9~16位)、字典饱和提前重置、
 *       编码/解码流式处理和压缩率统计。
 *
 * 协作: HuffmanTree7(哈夫曼) / RunLengthEncode5(RLE) / ArithmeticCoder3(算术编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief LZW编解码器
 */
class LempelZivWelch3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        double avgCompressionRatio = 0.0;///< 平均压缩率
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        int lastDictSize = 0;           ///< 最近字典大小
        int dictionaryResets = 0;       ///< 字典重置次数
    };

    explicit LempelZivWelch3(QObject *parent = nullptr);
    ~LempelZivWelch3() override;

    void setMinCodeBits(int bits);
    void setMaxCodeBits(int bits);
    void setEarlyResetEnabled(bool enabled);

    /**
     * @brief 编码(压缩)
     * @param input 输入字节流
     * @return 编码后的码字序列
     */
    QVector<int> encode(const QByteArray& input);

    /**
     * @brief 解码(解压)
     * @param codes 码字序列
     * @return 解码后的字节流
     */
    QByteArray decode(const QVector<int>& codes);

    /** @brief 编码为字节流 */
    QByteArray encodeToBytes(const QByteArray& input);

    /** @brief 从字节流解码 */
    QByteArray decodeFromBytes(const QByteArray& packed);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int originalSize, int compressedSize);
    void decodeCompleted(int outputSize);
    void dictionaryReset(int resetCount);

private:
    /** @brief 初始化字典(256个单字节条目) */
    void initDictionary();

    /** @brief 检查是否需要重置字典 */
    bool shouldReset(int dictSize) const;

    int m_minBits = 9;
    int m_maxBits = 16;
    bool m_earlyReset = true;

    Stats m_stats;
    double m_timeSum = 0.0;
};

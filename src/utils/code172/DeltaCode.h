/**
 * @file DeltaCode.h
 * @brief Delta编解码器(可配置字长+XOR/PCM差分模式) — Delta Encoder/Decoder with Configurable Word Size and XOR/PCM Delta Modes
 *
 * 功能: 实现Delta编码/解码，支持XOR差分和PCM差分模式、
 *       可配置字长(8/16/32位)、编解码统计。
 *
 * 协作: RunLengthCode3(游程编码) / HuffmanCode4(哈夫曼编码) / LZ77Compressor(LZ77)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Delta编解码器
 */
class DeltaCode : public QObject {
    Q_OBJECT

public:
    /** @brief Delta模式 */
    enum DeltaMode { XorDelta = 0, PcmDelta = 1 };

    /** @brief 字长 */
    enum WordSize { Bits8 = 8, Bits16 = 16, Bits32 = 32 };

    /** @brief 编解码统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        quint64 totalBytesIn = 0;       ///< 累计输入字节
        quint64 totalBytesOut = 0;      ///< 累计输出字节
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit DeltaCode(QObject *parent = nullptr);
    ~DeltaCode() override;

    void setMode(DeltaMode mode);
    void setWordSize(WordSize ws);

    /**
     * @brief Delta编码
     * @param input 输入字节数组
     * @return 编码后的字节数组
     */
    QByteArray encode(const QByteArray& input);

    /**
     * @brief Delta解码
     * @param input 编码后的字节数组
     * @return 解码后的字节数组
     */
    QByteArray decode(const QByteArray& input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int bytesIn, int bytesOut);
    void decodeCompleted(int bytesIn, int bytesOut);

private:
    /** @brief XOR delta encode for 8-bit */
    QByteArray encodeXor8(const QByteArray& input);

    /** @brief XOR delta decode for 8-bit */
    QByteArray decodeXor8(const QByteArray& input);

    /** @brief PCM delta encode for 8-bit */
    QByteArray encodePcm8(const QByteArray& input);

    /** @brief PCM delta decode for 8-bit */
    QByteArray decodePcm8(const QByteArray& input);

    /** @brief XOR delta encode for 16-bit */
    QByteArray encodeXor16(const QByteArray& input);

    /** @brief XOR delta decode for 16-bit */
    QByteArray decodeXor16(const QByteArray& input);

    /** @brief PCM delta encode for 16-bit */
    QByteArray encodePcm16(const QByteArray& input);

    /** @brief PCM delta decode for 16-bit */
    QByteArray decodePcm16(const QByteArray& input);

    /** @brief XOR delta encode for 32-bit */
    QByteArray encodeXor32(const QByteArray& input);

    /** @brief XOR delta decode for 32-bit */
    QByteArray decodeXor32(const QByteArray& input);

    /** @brief PCM delta encode for 32-bit */
    QByteArray encodePcm32(const QByteArray& input);

    /** @brief PCM delta decode for 32-bit */
    QByteArray decodePcm32(const QByteArray& input);

    DeltaMode m_mode = XorDelta;
    WordSize m_wordSize = Bits8;

    Stats m_stats;
    double m_timeSum = 0.0;
};

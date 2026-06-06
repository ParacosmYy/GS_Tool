/**
 * @file ArithmeticCode.h
 * @brief 算术编码(自适应概率模型+重归一化) — Arithmetic Encoder/Decoder with Adaptive Probability Models and Renormalization
 *
 * 功能: 实现算术编解码器，支持自适应频率模型、整数区间重归一化、
 *       按位编码/解码和字节级输入输出缓冲。
 *
 * 协作: HuffmanTree3(哈夫曼) / LZ77Compressor(LZ77) / RunLengthCode2(RLE)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 算术编解码器
 */
class ArithmeticCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;         ///< 累计编码次数
        quint64 totalDecodes = 0;         ///< 累计解码次数
        quint64 totalBitsEncoded = 0;     ///< 累计编码比特数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        double lastCompressionRatio = 0.0;///< 最近压缩率
    };

    /** @brief 符号频率表(自适应模型) */
    struct FrequencyModel {
        QVector<quint32> freq;     ///< 各符号频率
        quint32 total;             ///< 总频率
        quint32 maxSymbol;         ///< 最大符号值

        void reset(quint32 maxSym);
        void update(quint32 symbol);
        quint32 cumulative(quint32 symbol) const;
    };

    explicit ArithmeticCode(quint32 maxSymbol = 255, QObject *parent = nullptr);
    ~ArithmeticCode() override;

    void setMaxSymbol(quint32 maxSym);

    /**
     * @brief 编码符号序列
     * @param symbols 输入符号序列
     * @return 编码后的字节流
     */
    QByteArray encode(const QVector<quint32>& symbols);

    /**
     * @brief 解码字节流
     * @param data 编码数据
     * @param count 期望解码的符号数
     * @return 解码后的符号序列
     */
    QVector<quint32> decode(const QByteArray& data, int count);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param bits 输出比特数 */
    void encodeCompleted(quint64 bits);
    /** @brief 解码完成 @param count 符号数 */
    void decodeCompleted(int count);

private:
    /** @brief 编码器状态 */
    struct EncoderState {
        quint32 low;
        quint32 high;
        quint32 pendingBits;
        QByteArray output;
        void init();
    };

    /** @brief 解码器状态 */
    struct DecoderState {
        quint32 low;
        quint32 high;
        quint32 code;
        const char* data;
        int dataLen;
        int bytePos;
        int bitPos;
        void init(const QByteArray& input);
        quint32 readBit();
    };

    /** @brief 编码单个符号 */
    void encodeSymbol(EncoderState& st, quint32 symbol, FrequencyModel& model);

    /** @brief 解码单个符号 */
    quint32 decodeSymbol(DecoderState& st, FrequencyModel& model);

    /** @brief 编码器重归一化 */
    void encodeRenormalize(EncoderState& st);

    /** @brief 解码器重归一化 */
    void decodeRenormalize(DecoderState& st);

    /** @brief 输出一个bit */
    void outputBit(EncoderState& st, quint32 bit);

    /** @brief 编码器flush */
    void flushEncoder(EncoderState& st);

    static constexpr quint32 kPrecision = 32;
    static constexpr quint32 kWhole = 0x100000000ULL;
    static constexpr quint32 kHalf  = 0x80000000ULL;
    static constexpr quint32 kQuarter = 0x40000000ULL;
    static constexpr quint32 kThreeQuarter = 0xC0000000ULL;
    static constexpr quint32 kMask = 0xFFFFFFFFULL;

    quint32 m_maxSymbol = 255;

    Stats m_stats;
    double m_timeSum = 0.0;
};

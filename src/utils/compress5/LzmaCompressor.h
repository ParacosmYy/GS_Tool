/**
 * @file LzmaCompressor.h
 * @brief LZMA风格压缩器 — 基于范围编码器的LZ压缩
 *
 * 功能: 实现LZMA风格的压缩/解压缩，使用范围编码器(Range Coder)
 *       作为后端，结合LZ77滑动窗口匹配和自适应概率模型。
 *       适用于嵌入式数据传输中的压缩场景。
 *
 * 协作: AesCbc(加密前压缩) / DataRecorder(数据存储压缩)
 */
#pragma once

#include <QObject>
#include <QByteArray>

#include <cstdint>
#include <vector>

/**
 * @brief LZMA风格压缩器 — 范围编码器 + LZ77匹配
 */
class LzmaCompressor : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalCompressions = 0;        ///< 累计压缩次数
        int totalDecompressions = 0;      ///< 累计解压次数
        int totalBytesIn = 0;             ///< 累计输入字节数
        int totalBytesOut = 0;            ///< 累计输出字节数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param windowBits 滑动窗口位数(默认16=64KB窗口)
     * @param parent 父对象
     */
    explicit LzmaCompressor(int windowBits = 16, QObject* parent = nullptr);

    /**
     * @brief 压缩数据
     * @param input 输入数据
     * @return 压缩后数据(空表示失败)
     */
    QByteArray compress(const QByteArray& input);

    /**
     * @brief 解压数据
     * @param compressed 压缩数据
     * @param originalSize 原始数据大小
     * @return 解压后数据(空表示失败)
     */
    QByteArray decompress(const QByteArray& compressed, int originalSize);

    /**
     * @brief 获取压缩率
     * @return 最近一次压缩的压缩比(输出/输入)
     */
    double lastCompressionRatio() const { return m_lastRatio; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 压缩完成 @param inSize 输入大小 @param outSize 输出大小 @param ratio 压缩比 */
    void compressionCompleted(int inSize, int outSize, double ratio);

    /** @brief 解压完成 @param outSize 输出大小 */
    void decompressionCompleted(int outSize);

private:
    /**
     * @brief 范围编码器
     */
    struct RangeEncoder {
        uint32_t low = 0;           ///< 编码下界
        uint32_t range = 0xFFFFFFFF;///< 编码范围
        QByteArray output;          ///< 输出缓冲区

        void init();
        void encodeBit(int bit, uint32_t& prob);
        void encodeDirectBits(uint32_t value, int numBits);
        void flush();
    };

    /**
     * @brief 范围解码器
     */
    struct RangeDecoder {
        uint32_t code = 0;          ///< 当前编码值
        uint32_t range = 0xFFFFFFFF;///< 编码范围
        const char* data = nullptr; ///< 输入数据指针
        int dataSize = 0;           ///< 数据大小
        int pos = 0;                ///< 当前位置

        void init(const char* src, int size);
        int decodeBit(uint32_t& prob);
        uint32_t decodeDirectBits(int numBits);
    };

    /**
     * @brief 在滑动窗口中查找最长匹配
     * @param data 输入数据
     * @param pos 当前位置
     * @param matchLen 输出匹配长度
     * @return 匹配偏移(0=无匹配)
     */
    int findMatch(const QByteArray& data, int pos, int& matchLen) const;

    /** @brief 编码一个字面量字节 @param enc 编码器 @param byte 字节 */
    void encodeLiteral(RangeEncoder& enc, uint8_t byte);

    /** @brief 编码匹配 @param enc 编码器 @param offset 偏移 @param length 长度 */
    void encodeMatch(RangeEncoder& enc, int offset, int length);

    /** @brief 解码一个token @param dec 解码器 @return (isMatch, literal_or_offset, length) */
    std::tuple<bool, uint8_t, int, int> decodeToken(RangeDecoder& dec);

    int m_windowBits;               ///< 窗口位数
    int m_windowSize;               ///< 窗口大小
    int m_minMatchLen = 3;          ///< 最小匹配长度
    int m_maxMatchLen = 258;        ///< 最大匹配长度
    double m_lastRatio = 0.0;       ///< 最近压缩比

    uint32_t m_literalProb[256];    ///< 字面量概率
    uint32_t m_matchProb = 1024;    ///< 匹配概率
    uint32_t m_lenProb[16];         ///< 长度概率
    uint32_t m_distProb[16];        ///< 距离概率

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};

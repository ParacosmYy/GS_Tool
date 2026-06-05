/**
 * @file LzwEncoder.h
 * @brief LZW编码/解码器 — 可变位宽字典压缩
 *
 * 实现Lempel-Ziv-Welch压缩算法, 支持可变位宽输出、
 * 字典大小控制、以及完整的编码/解码功能。
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

/**
 * @class LzwEncoder
 * @brief LZW编码/解码器 — 可变位宽字典压缩
 *
 * 编码: 输入字节流 → LZW码字流(可变位宽打包)
 * 解码: LZW码字流 → 原始字节流
 * 支持自定义初始字典大小和最大字典条目数。
 */
class LzwEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 总编码次数
        quint64 totalDecodes = 0;       ///< 总解码次数
        quint64 totalBytesIn = 0;       ///< 总输入字节数
        quint64 totalBytesOut = 0;      ///< 总输出字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LzwEncoder(QObject* parent = nullptr);

    /**
     * @brief 设置最大字典大小
     * @param maxDictSize 最大条目数(默认4096)
     */
    void setMaxDictSize(int maxDictSize);

    /**
     * @brief 设置初始位宽
     * @param bitWidth 初始码字位宽(8~16, 默认9)
     */
    void setInitialBitWidth(int bitWidth);

    /**
     * @brief 编码数据
     * @param data 输入数据
     * @return LZW压缩后的字节数组
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码数据
     * @param compressed LZW压缩数据
     * @return 解码后的原始数据
     */
    QByteArray decode(const QByteArray& compressed);

    /**
     * @brief 获取压缩率
     * @param original 原始大小
     * @param compressed 压缩后大小
     * @return 压缩率(0~1, 越小越好)
     */
    double compressionRatio(int originalSize, int compressedSize) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param original 原始大小 @param compressed 压缩大小 */
    void encodeCompleted(int original, int compressed);
    /** @brief 解码完成 @param compressed 压缩大小 @param decompressed 解码大小 */
    void decodeCompleted(int compressed, int decompressed);

private:
    /** @brief 位写入器: 将可变位宽码字打包到字节数组 */
    void writeBits(QByteArray& output, int code, int bitWidth, int& bitBuffer, int& bitCount) const;

    /** @brief 位读取器: 从字节数组读取可变位宽码字 */
    int readBits(const QByteArray& input, int bitWidth, int& bytePos, int& bitBuffer, int& bitCount) const;

    int m_maxDictSize = 4096;   ///< 最大字典大小
    int m_initialBitWidth = 9;  ///< 初始码字位宽

    Stats m_stats;              ///< 操作统计
    double m_timeSum = 0.0;     ///< 累计耗时
};

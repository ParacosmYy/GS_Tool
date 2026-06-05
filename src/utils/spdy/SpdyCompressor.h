/**
 * @file SpdyCompressor.h
 * @brief SPDY头部压缩器(SPDY Header Compressor)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>

/**
 * @class SpdyCompressor
 * @brief SPDY头部压缩器 — 基于字典的HTTP头部压缩
 *
 * 支持静态/动态字典、哈夫曼编码、头部索引。
 * 适用于HTTP/2 HPACK风格压缩、协议优化等场景。
 */
class SpdyCompressor : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalCompressed = 0;   /**< 总压缩次数 */
        int totalDecompressed = 0; /**< 总解压次数 */
        long long totalBytesIn = 0;  /**< 总输入字节 */
        long long totalBytesOut = 0; /**< 总输出字节 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit SpdyCompressor(int dynamicTableSize = 4096,
                              QObject* parent = nullptr);

    /**
     * @brief 压缩头部
     * @param headers 键值对
     * @return 压缩后数据
     */
    QByteArray compress(const QMap<QString, QString>& headers);

    /**
     * @brief 解压头部
     * @param data 压缩数据
     * @return 键值对
     */
    QMap<QString, QString> decompress(const QByteArray& data);

    /**
     * @brief 编码整数(可变长度)
     * @param value 值
     * @param prefixBits 前缀比特数
     * @return 编码后字节
     */
    static QByteArray encodeInteger(int value, int prefixBits);

    /**
     * @brief 解码整数
     * @param data 数据
     * @param offset 起始偏移
     * @param prefixBits 前缀比特数
     * @return (值, 消耗字节数)
     */
    static QPair<int, int> decodeInteger(const QByteArray& data, int offset,
                                            int prefixBits);

    /** @brief 压缩率 */
    double compressionRatio() const;

    /** @brief 动态表大小 */
    int dynamicTableSize() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 */
    void compressionCompleted(int originalSize, int compressedSize);

private:
    int m_maxTableSize;
    QMap<QString, int> m_staticTable;
    QVector<QPair<QString, QString>> m_dynamicTable;
    int m_dynamicTableSize;

    Stats m_stats;
    double m_timeSum;
};

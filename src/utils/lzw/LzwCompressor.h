/**
 * @file LzwCompressor.h
 * @brief LZW压缩器 — Lempel-Ziv-Welch字典压缩
 *
 * 功能: 基于字典的LZW压缩/解压，可配置最大字典大小，
 *       统计压缩/解压次数和平均压缩率。
 */
#ifndef LZWCOMPRESSOR_H
#define LZWCOMPRESSOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

/**
 * @class LzwCompressor
 * @brief LZW字典压缩算法
 */
class LzwCompressor : public QObject {
    Q_OBJECT
public:
    /** 压缩统计 */
    struct Stats {
        quint64 totalCompressions = 0;
        quint64 totalDecompressions = 0;
        double  avgCompressionRatio = 0.0;
        quint64 totalBytesIn = 0;
        quint64 totalBytesOut = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit LzwCompressor(QObject* parent = nullptr);

    void setMaxDictSize(int maxSize);

    /** 压缩 */
    QByteArray compress(const QByteArray& data);

    /** 解压 */
    QByteArray decompress(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void compressComplete(int originalSize, int compressedSize, double ratio);
    void decompressComplete(int compressedSize, int decompressedSize);

private:
    int m_maxDictSize;
    Stats m_stats;
    double m_timeSum;
};

#endif // LZWCOMPRESSOR_H

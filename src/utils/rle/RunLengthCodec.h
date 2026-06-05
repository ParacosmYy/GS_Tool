/**
 * @file RunLengthCodec.h
 * @brief 行程编码器 — RLE压缩/解压
 *
 * 功能: 可配置标记字节的行程编码，处理二进制和文本数据，
 *       统计编码/解码次数/压缩率。
 */
#ifndef RUNLENGTHCODEC_H
#define RUNLENGTHCODEC_H

#include <QObject>
#include <QByteArray>

/**
 * @class RunLengthCodec
 * @brief 行程编码(RLE)压缩器
 */
class RunLengthCodec : public QObject {
    Q_OBJECT
public:
    /** 编码统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        quint64 totalBytesIn = 0;
        quint64 totalBytesOut = 0;
        double  compressionRatio = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit RunLengthCodec(QObject* parent = nullptr);

    void setMinRunLength(int minRun);
    void setMarkerByte(quint8 marker);

    /** 编码 */
    QByteArray encode(const QByteArray& data);

    /** 解码 */
    QByteArray decode(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeComplete(int originalSize, int compressedSize, double ratio);
    void decodeComplete(int compressedSize, int decompressedSize);

private:
    int m_minRunLength;
    quint8 m_markerByte;
    Stats m_stats;
    double m_timeSum;
};

#endif // RUNLENGTHCODEC_H

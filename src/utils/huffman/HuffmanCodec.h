/**
 * @file HuffmanCodec.h
 * @brief Huffman编码器 — 基于频率的最优前缀编码
 *
 * 功能: 从频率表构建Huffman树，编码/解码比特流，
 *       统计编码次数和平均压缩率。
 */
#ifndef HUFFMANCODEC_H
#define HUFFMANCODEC_H

#include <QObject>
#include <QByteArray>
#include <QMap>

/**
 * @class HuffmanCodec
 * @brief Huffman编码/解码器
 */
class HuffmanCodec : public QObject {
    Q_OBJECT
public:
    /** 编码统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        double  avgCompressionRatio = 0.0;
        quint64 totalBytesIn = 0;
        quint64 totalBytesOut = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit HuffmanCodec(QObject* parent = nullptr);

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
    struct HuffNode {
        quint8 byte;
        int freq;
        HuffNode* left;
        HuffNode* right;
        HuffNode(quint8 b, int f) : byte(b), freq(f), left(nullptr), right(nullptr) {}
    };

    HuffNode* buildTree(const QMap<quint8, int>& freq);
    void buildCodes(HuffNode* node, const QByteArray& prefix, QMap<quint8, QByteArray>& codes);
    void deleteTree(HuffNode* node);

    Stats m_stats;
    double m_timeSum;
};

#endif // HUFFMANCODEC_H

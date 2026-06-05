/**
 * @file Lz78Compressor.h
 * @brief LZ78压缩引擎 — 基于字典的压缩与解压
 *
 * 功能: 实现LZ78压缩算法，动态构建字典进行数据压缩与解压，
 *       统计压缩/解压次数、输入输出字节数及平均处理耗时。
 *
 * 协作: DataCompressor(通用压缩) / DataExporter(压缩导出)
 */
#ifndef LZ78COMPRESSOR_H
#define LZ78COMPRESSOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>

/**
 * @brief LZ78压缩/解压引擎
 */
class Lz78Compressor : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalCompressed = 0;        ///< 累计压缩次数
        quint64 totalDecompressed = 0;      ///< 累计解压次数
        quint64 totalBytesIn = 0;           ///< 累计输入字节数
        quint64 totalBytesOut = 0;          ///< 累计输出字节数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit Lz78Compressor(QObject* parent = nullptr);

    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @return 压缩后的数据
     */
    QByteArray compress(const QByteArray& data);

    /**
     * @brief 解压数据
     * @param data 压缩数据
     * @return 解压后的原始数据
     */
    QByteArray decompress(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 压缩完成信号 @param originalSize 原始大小 @param compressedSize 压缩后大小 */
    void compressionCompleted(int originalSize, int compressedSize);

private:
    double m_timeSum;           ///< 累计耗时(ms)
    mutable Stats m_stats;      ///< 可变统计
};

#endif // LZ78COMPRESSOR_H

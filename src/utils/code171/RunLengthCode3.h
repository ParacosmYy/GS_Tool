/**
 * @file RunLengthCode3.h
 * @brief 游程编码(PackBits变体+多通道支持) — Run-Length Encoder/Decoder with PackBits Variant and Multi-Channel Support
 *
 * 功能: 实现游程编码压缩与解压，支持PackBits变体、
 *       多通道数据分离编解码和自适应阈值控制。
 *
 * 协作: HuffmanCode2(哈夫曼) / LzwCode3(LZW) / DeltaCode4(差分编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 游程编码器
 */
class RunLengthCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief 编码模式 */
    enum Mode { Basic, PackBits };

    /** @brief 运程项 */
    struct Run {
        int count = 0;    ///< 重复次数(正)或字面量计数(负)
        qint8 value = 0;  ///< 值(仅重复项有效)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        double avgCompressionRatio = 0.0;///< 平均压缩率
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
    };

    explicit RunLengthCode3(QObject *parent = nullptr);
    ~RunLengthCode3() override;

    void setMode(Mode mode);
    void setMaxRunLength(int maxRun);

    /**
     * @brief 编码(单通道)
     * @param input 原始字节序列
     * @return 编码后的字节序列
     */
    QByteArray encode(const QByteArray& input) const;

    /**
     * @brief 解码(单通道)
     * @param encoded 编码后的字节序列
     * @return 解码后的原始字节序列
     */
    QByteArray decode(const QByteArray& encoded) const;

    /**
     * @brief 多通道编码
     * @param channels 各通道数据
     * @return 各通道编码结果
     */
    QVector<QByteArray> encodeMultiChannel(
        const QVector<QByteArray>& channels) const;

    /**
     * @brief 多通道解码
     * @param encoded 各通道编码数据
     * @return 各通道解码结果
     */
    QVector<QByteArray> decodeMultiChannel(
        const QVector<QByteArray>& encoded) const;

    /** @brief 分析编码效率 */
    double compressionRatio(const QByteArray& input,
                            const QByteArray& encoded) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inputSize, int outputSize);
    void decodeCompleted(int outputSize);

private:
    /** @brief PackBits编码 */
    QByteArray encodePackBits(const QByteArray& input) const;

    /** @brief PackBits解码 */
    QByteArray decodePackBits(const QByteArray& encoded) const;

    /** @brief 基本游程编码 */
    QByteArray encodeBasic(const QByteArray& input) const;

    /** @brief 基本游程解码 */
    QByteArray decodeBasic(const QByteArray& encoded) const;

    Mode m_mode = PackBits;
    int m_maxRun = 128;

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;
};

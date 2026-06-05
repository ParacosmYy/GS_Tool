/**
 * @file RleEncoder.h
 * @brief RLE(游程编码)增强版
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class RleEncoder
 * @brief RLE游程编码 — 支持多种RLE变体和二进制/文本模式
 *
 * 支持经典RLE、PackBits编码和自定义控制字。
 * 适用于位图压缩、简单信号压缩等场景。
 */
class RleEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief RLE变体 */
    enum Mode {
        Classic,    /**< 经典RLE: [count][byte] */
        PackBits,   /**< PackBits: 控制字+数据 */
        HeaderLess  /**< 无头RLE: 直接计数+值交替 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;       /**< 总编码次数 */
        int totalDecoded = 0;       /**< 总解码次数 */
        int totalBytesIn = 0;       /**< 输入字节数 */
        int totalBytesOut = 0;      /**< 输出字节数 */
        double avgCompressionRatio = 0.0; /**< 平均压缩比 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit RleEncoder(QObject* parent = nullptr);

    /**
     * @brief 编码
     * @param data 输入数据
     * @param mode RLE模式
     * @return 编码后数据
     */
    QByteArray encode(const QByteArray& data, Mode mode = Classic) const;

    /**
     * @brief 解码
     * @param data 编码数据
     * @param mode RLE模式
     * @return 原始数据
     */
    QByteArray decode(const QByteArray& data, Mode mode = Classic) const;

    /**
     * @brief 编码QVector<double>
     * @param values 数值序列
     * @return (值列表, 计数列表)
     */
    QPair<QVector<double>, QVector<int>> encodeValues(
        const QVector<double>& values) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodeCompleted(int inSize, int outSize, double ratio);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

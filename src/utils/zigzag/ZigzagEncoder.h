/**
 * @file ZigzagEncoder.h
 * @brief ZigZag编码器(有符号整数↔无符号整数)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class ZigzagEncoder
 * @brief ZigZag编码 — 将有符号整数映射为无符号整数以优化变长编码
 *
 * ZigZag编码将小绝对值的有符号整数映射为小无符号整数，
 * 适用于Protocol Buffers等场景的变长整数编码前置处理。
 */
class ZigzagEncoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;       /**< 总编码数 */
        int totalDecoded = 0;       /**< 总解码数 */
        int totalEncodedBytes = 0;  /**< 编码总字节数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ZigzagEncoder(QObject* parent = nullptr);

    /** @brief 编码: 有符号32位 → 无符号32位 */
    quint32 encode32(qint32 value) const;

    /** @brief 解码: 无符号32位 → 有符号32位 */
    qint32 decode32(quint32 value) const;

    /** @brief 编码: 有符号64位 → 无符号64位 */
    quint64 encode64(qint64 value) const;

    /** @brief 解码: 无符号64位 → 有符号64位 */
    qint64 decode64(quint64 value) const;

    /** @brief 批量编码32位 */
    QVector<quint32> encodeBatch32(const QVector<qint32>& values) const;

    /** @brief 批量解码32位 */
    QVector<qint32> decodeBatch32(const QVector<quint32>& values) const;

    /** @brief 编码为变长字节(Varint + ZigZag) */
    QByteArray encodeVarint(qint64 value) const;

    /** @brief 从变长字节解码 */
    qint64 decodeVarint(const QByteArray& data, int* bytesRead = nullptr) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodeCompleted(int count);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};

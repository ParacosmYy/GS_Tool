/**
 * @file ReedSolomon3.h
 * @brief Reed-Solomon纠删编码引擎 — 基于GF(2^8)的数据恢复
 *
 * 功能: 在GF(2^8)伽罗瓦域上实现Reed-Solomon编码，支持数据
 *       纠删(恢复丢失数据分片)。适用于串口数据包的冗余编码、
 *       OTA固件分片的纠删保护、多通道数据的容错传输。
 *
 * 协作: DataCompressor(压缩后编码) / StreamCaptureRecorder(录制保护)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QList>

/**
 * @brief Reed-Solomon纠删编码引擎 — GF(2^8)域运算
 */
class ReedSolomon3 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncodes = 0;                  ///< 累计编码次数
        int totalDecodes = 0;                  ///< 累计解码次数
        int totalRecoveredShards = 0;          ///< 累计恢复分片数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ReedSolomon3(QObject* parent = nullptr);

    /**
     * @brief 编码: 将数据分片并生成校验分片
     * @param data 原始数据
     * @param dataShards 数据分片数
     * @param parityShards 校验分片数
     * @return 所有分片(数据分片 + 校验分片)
     */
    QList<QByteArray> encode(const QByteArray& data,
                              int dataShards,
                              int parityShards);

    /**
     * @brief 解码/恢复: 从可用分片恢复原始数据
     * @param shards 分片数组(丢失位置用空QByteArray表示)
     * @param dataShards 数据分片数
     * @param parityShards 校验分片数
     * @param shardSize 每个分片的大小
     * @return 恢复后的原始数据
     */
    QByteArray decode(const QList<QByteArray>& shards,
                      int dataShards,
                      int parityShards,
                      int shardSize);

    /**
     * @brief GF(2^8)乘法
     * @param a 操作数a
     * @param b 操作数b
     * @return a * b (mod不可约多项式)
     */
    static quint8 gfMul(quint8 a, quint8 b);

    /**
     * @brief GF(2^8)除法
     * @param a 被除数
     * @param b 除数
     * @return a / b
     */
    static quint8 gfDiv(quint8 a, quint8 b);

    /**
     * @brief GF(2^8)求逆
     * @param a 输入值
     * @return a的乘法逆元
     */
    static quint8 gfInv(quint8 a);

    /**
     * @brief GF(2^8)指数运算
     * @param a 底数
     * @param n 指数
     * @return a^n
     */
    static quint8 gfPow(quint8 a, int n);

    /**
     * @brief 生成Vandermonde编码矩阵
     * @param rows 行数(总分片数)
     * @param cols 列数(数据分片数)
     * @return 编码矩阵
     */
    QVector<QVector<quint8>> buildMatrix(int rows, int cols);

    /**
     * @brief 获取统计信息
     * @return 统计引用
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 编码完成
     * @param dataShards 数据分片数
     * @param totalShards 总分片数
     */
    void encodeCompleted(int dataShards, int totalShards);

    /**
     * @brief 解码完成
     * @param recoveredCount 恢复的分片数
     * @param dataLength 恢复数据长度
     */
    void decodeCompleted(int recoveredCount, int dataLength);

private:
    /**
     * @brief GF(2^8)加法(异或)
     * @param a 操作数a
     * @param b 操作数b
     * @return a + b
     */
    static quint8 gfAdd(quint8 a, quint8 b);

    /**
     * @brief 矩阵求逆(在GF(2^8)上Gauss-Jordan消元)
     * @param matrix 输入方阵
     * @return 逆矩阵; 空表示奇异
     */
    QVector<QVector<quint8>> invertMatrix(
        const QVector<QVector<quint8>>& matrix);

    Stats m_stats;
    double m_timeSum = 0.0;             ///< 处理时间累加器
};

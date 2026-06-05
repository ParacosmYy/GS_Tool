/**
 * @file EchoHash.h
 * @brief Echo 哈希计算引擎(AES-Based)
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 基于 AES 轮模拟和 Feistel 混合的非加密哈希函数。
 * 提供 64 位和 128 位哈希输出。
 */

#ifndef ECHOHASH_H
#define ECHOHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QtGlobal>

/**
 * @class EchoHash
 * @brief Echo 哈希计算器，基于 AES 轮模拟
 *
 * 使用 AES S-box 替换 + Feistel 结构混合实现快速哈希。
 * 输出 64 位或 128 位(QPair<quint64,quint64>)哈希值。
 */
class EchoHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;       ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理的字节总数
        quint64 totalRounds = 0;       ///< 累计 AES 轮执行次数
        double avgProcessingTimeMs = 0.0; ///< 平均每次哈希处理耗时(毫秒)
    };

    /** @brief 构造 Echo 哈希计算器 @param parent 父对象 */
    explicit EchoHash(QObject *parent = nullptr);

    /** @brief 设置哈希种子值 @param seed 64位种子 */
    void setSeed(quint64 seed);

    /** @brief 计算64位哈希值 @param data 输入数据 @return 64位哈希值 */
    quint64 hash64(const QByteArray &data);

    /** @brief 计算128位哈希值 @param data 输入数据 @return QPair(高64位, 低64位) */
    QPair<quint64, quint64> hash128(const QByteArray &data);

    /** @brief 获取统计信息 @return 统计结构的常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief AES S-box 替换 @param byte 输入字节 @return S-box 输出字节 */
    static quint8 aesSbox(quint8 byte);

    /** @brief AES SubBytes 步: 对64位值进行 S-box 替换 @param val 输入值 @return 替换结果 */
    static quint64 subBytes64(quint64 val);

    /** @brief Feistel 混合轮 @param left 左半部分 @param right 右半部分 @param roundKey 轮密钥 */
    void feistelRound(quint64 &left, quint64 &right, quint64 roundKey);

    /** @brief 对一个64位块进行完整哈希处理 @param block 数据块 @param blockIndex 块索引 @return 哈希值 */
    quint64 hashBlock64(const quint8 *block, int size, quint64 blockIndex);

    /** @brief 生成轮密钥 @param round 轮次 @return 轮密钥 */
    quint64 deriveRoundKey(int round) const;

    quint64 m_seed;             ///< 种子值
    Stats m_stats;              ///< 统计信息
    QElapsedTimer m_timer;      ///< 计时器
    double m_accumulatedTimeMs; ///< 累计处理时间(毫秒)
};

#endif // ECHOHASH_H

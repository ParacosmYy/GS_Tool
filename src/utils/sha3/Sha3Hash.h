/**
 * @file Sha3Hash.h
 * @brief SHA-3 (Keccak) 哈希计算引擎
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 基于简化的 Keccak-f[1600] 置换实现 SHA-3 哈希。
 * 支持 224/256/384/512 位摘要长度。
 */

#ifndef SHA3HASH_H
#define SHA3HASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class Sha3Hash
 * @brief SHA-3 哈希计算器，基于 Keccak 海绵构造
 *
 * 流式接口: init() -> update()* -> finalize()
 * 一次性接口: static hash()
 */
class Sha3Hash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;       ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理的字节总数
        quint64 totalUpdates = 0;       ///< 累计 update() 调用次数
        double avgProcessingTimeMs = 0.0; ///< 平均每次哈希处理耗时(毫秒)
    };

    /** @brief 构造 SHA-3 哈希计算器 @param parent 父对象 */
    explicit Sha3Hash(QObject *parent = nullptr);

    /** @brief 初始化哈希状态 @param hashBits 输出摘要位数(224/256/384/512)，默认256 */
    void init(int hashBits = 256);

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray &data);

    /** @brief 完成哈希计算并返回摘要 @return 哈希摘要字节数组 */
    QByteArray finalize();

    /** @brief 一次性计算完整数据的 SHA-3 哈希 @param data 输入数据 @param bits 摘要位数 @return 哈希摘要 */
    static QByteArray hash(const QByteArray &data, int bits = 256);

    /** @brief 获取统计信息 @return 统计结构的常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param bits 摘要位数 @param inputSize 输入数据大小 */
    void hashComputed(int bits, qint64 inputSize);

private:
    /** @brief Keccak-f[1600] 置换，24轮 */
    void keccakF();

    /** @brief Theta 步: 列奇偶校验扩散 */
    void theta();

    /** @brief Rho 步: 位平面旋转 */
    void rho();

    /** @brief Pi 步: 位置置换 */
    void pi();

    /** @brief Chi 步: 非线性变换 */
    void chi();

    /** @brief Iota 步: 轮常数异或 @param round 当前轮次 */
    void iota(int round);

    /** @brief 将数据吸收到海绵状态中 @param data 吸收数据 @param offset 偏移量 @param length 长度 */
    void absorb(const quint8 *data, int offset, int length);

    /** @brief 挤出哈希摘要 @param digestBytes 需要挤出的字节数 @return 摘要字节数组 */
    QByteArray squeeze(int digestBytes);

    /** @brief 计算海绵容量(比特) @return 容量值 */
    int capacity() const;

    /** @brief 计算海绵速率(字节) @return 速率值 */
    int rateBytes() const;

    quint64 m_state[25];       ///< Keccak 5x5 状态矩阵(每个64位)
    quint8 m_buffer[200];      ///< 吸收缓冲区
    int m_bufferPos;           ///< 缓冲区当前位置
    int m_hashBits;            ///< 输出摘要位数
    bool m_finalized;          ///< 是否已完成 finalize
    qint64 m_totalInputSize;   ///< 当前哈希累计输入大小

    Stats m_stats;             ///< 统计信息
    QElapsedTimer m_timer;     ///< 计时器
    double m_accumulatedTimeMs; ///< 累计处理时间(毫秒)
};

#endif // SHA3HASH_H

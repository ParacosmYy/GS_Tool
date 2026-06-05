/**
 * @file KeccakHash.h
 * @brief SHA-3 / Keccak 海绵构造哈希算法
 *
 * 实现 Keccak sponge 构造，支持 SHA-3 标准哈希输出
 * (224/256/384/512 位)。可用于数据完整性校验、
 * 嵌入式固件签名验证等场景。
 *
 * 协作: ChecksumCalculator(校验) / DataEncryptionEngine(加密)
 */
#ifndef KECCAKHASH_H
#define KECCAKHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class KeccakHash
 * @brief SHA-3 / Keccak 海绵构造哈希引擎
 */
class KeccakHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;           ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0;   ///< 累计处理字节数
        quint64 totalUpdates = 0;          ///< 累计 update 调用次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit KeccakHash(QObject* parent = nullptr);

    /** @brief 初始化哈希状态 @param hashBits 输出位宽(224/256/384/512) */
    void init(int hashBits = 256);

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray& data);

    /** @brief 完成哈希计算并返回摘要 @return 哈希摘要字节串 */
    QByteArray finalize();

    /**
     * @brief 一次性计算哈希(便捷接口)
     * @param data 输入数据
     * @param bits 输出位宽(224/256/384/512)
     * @return 哈希摘要字节串
     */
    QByteArray hash(const QByteArray& data, int bits = 256);

    /** @brief 获取统计信息 @return 统计结构体的常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param bits 输出位宽 @param inputSize 输入数据大小 */
    void hashComputed(int bits, int inputSize);

private:
    /** @brief Keccak-f[b] 置换函数(1600位状态) */
    void keccakF();
    /** @brief 海绵吸收阶段 @param input 输入数据 @param offset 偏移量 @param length 数据长度 */
    void absorb(const QByteArray& input, int offset, int length);
    /** @brief 海绵挤压阶段 @param output 输出缓冲 @param outputBytes 输出字节数 */
    void squeeze(QByteArray& output, int outputBytes);
    /** @brief 对状态应用填充规则(Keccak的多位率填充) */
    void pad();

    static constexpr int STATE_SIZE = 200;   ///< Keccak-1600 状态字节大小
    static constexpr int LANE_SIZE = 8;      ///< 每条通道字节大小(64位)
    static constexpr int NUM_ROUNDS = 24;    ///< Keccak-f 置换轮数

    quint8 m_state[STATE_SIZE];              ///< Keccak 状态(200字节=1600位)
    int m_rate;                              ///< 吸收率(字节)
    int m_hashBits;                          ///< 输出位宽
    int m_absorbedBytes;                     ///< 已吸收字节数(在当前块中)
    bool m_finalized;                        ///< 是否已完成 finalize

    QByteArray m_buffer;                     ///< 内部缓冲区
    QElapsedTimer m_timer;                   ///< 计时器
    Stats m_stats;                           ///< 统计信息
    double m_timeSum;                        ///< 累计处理时间
};

#endif // KECCAKHASH_H

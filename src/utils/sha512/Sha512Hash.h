/**
 * @file Sha512Hash.h
 * @brief SHA-512 安全哈希计算引擎
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 完整实现 SHA-512 (FIPS 180-4) 安全哈希算法。
 * 流式接口: init() -> update()* -> finalize()
 * 一次性接口: static hash()
 */

#ifndef SHA512HASH_H
#define SHA512HASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class Sha512Hash
 * @brief SHA-512 哈希计算器，符合 FIPS 180-4 标准
 *
 * 支持 64 位平台优化的 80 轮消息摘要计算。
 */
class Sha512Hash : public QObject
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

    /** @brief 构造 SHA-512 哈希计算器 @param parent 父对象 */
    explicit Sha512Hash(QObject *parent = nullptr);

    /** @brief 初始化哈希状态，设置初始哈希值 */
    void init();

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray &data);

    /** @brief 完成哈希计算并返回64字节摘要 @return 512位(64字节)哈希摘要 */
    QByteArray finalize();

    /** @brief 一次性计算完整数据的 SHA-512 哈希 @param data 输入数据 @return 哈希摘要 */
    static QByteArray hash(const QByteArray &data);

    /** @brief 获取统计信息 @return 统计结构的常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief 处理一个1024位(128字节)消息块 @param block 消息块指针 */
    void processBlock(const quint8 *block);

    /** @brief 将 8 字节转为 quint64 (大端) @param data 数据指针 @return 64位值 */
    static quint64 toUInt64(const quint8 *data);

    quint64 m_state[8];        ///< 8 个 64 位哈希状态字
    quint8 m_buffer[128];      ///< 128字节消息缓冲区
    int m_bufferPos;           ///< 缓冲区当前位置
    quint64 m_totalBits;       ///< 累计处理的总位数
    bool m_finalized;          ///< 是否已完成 finalize
    qint64 m_totalInputSize;   ///< 当前哈希累计输入大小

    Stats m_stats;             ///< 统计信息
    QElapsedTimer m_timer;     ///< 计时器
    double m_accumulatedTimeMs; ///< 累计处理时间(毫秒)
};

#endif // SHA512HASH_H

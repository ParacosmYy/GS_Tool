/**
 * @file XxHash.h
 * @brief xxHash64 快速非加密哈希算法
 *
 * 实现 xxHash64 算法，支持流式和一次性哈希计算。
 * 极高的吞吐量适用于数据校验、哈希表索引、
 * 实时数据流指纹等场景。
 *
 * 协作: DataChecksumVerifier(校验) / RollingHash(滚动哈希)
 */
#ifndef XXHASH_H
#define XXHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class XxHash
 * @brief xxHash64 快速非加密哈希引擎
 */
class XxHash : public QObject
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
    explicit XxHash(QObject* parent = nullptr);

    /** @brief 初始化哈希状态 @param seed 种子值 */
    void init(quint64 seed = 0);

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray& data);

    /** @brief 完成哈希计算并返回64位摘要 @return 64位哈希值 */
    quint64 digest();

    /**
     * @brief 一次性计算哈希(静态便捷接口)
     * @param data 输入数据
     * @param seed 种子值
     * @return 64位哈希值
     */
    static quint64 hash(const QByteArray& data, quint64 seed = 0);

    /** @brief 获取统计信息 @return 统计结构体的常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief 处理一个 stripe(32字节) @param data 数据指针 */
    void processStripe(const char* data);
    /** @brief 合并累加器为最终哈希值 @return 64位结果 */
    quint64 mergeAccumulators() const;
    /** @brief 混合步骤(位旋转+乘法) @param lhs 值A @param rhs 值B */
    static quint64 mul128Fold64(quint64 lhs, quint64 rhs);

    static constexpr quint64 PRIME64_1 = 0x9E3779B185EBCA87ULL;
    static constexpr quint64 PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
    static constexpr quint64 PRIME64_3 = 0x165667B19E3779F9ULL;
    static constexpr quint64 PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
    static constexpr quint64 PRIME64_5 = 0x27D4EB2F165667C5ULL;
    static constexpr int STRIPE_LEN = 32;   ///< 每个 stripe 32字节

    quint64 m_acc[4];               ///< 4个累加器
    quint64 m_seed;                 ///< 种子值
    QByteArray m_buffer;            ///< 内部缓冲区
    bool m_finalized;               ///< 是否已完成 digest

    QElapsedTimer m_timer;          ///< 计时器
    Stats m_stats;                  ///< 统计信息
    double m_timeSum;               ///< 累计处理时间
};

#endif // XXHASH_H

/**
 * @file SpookyHash.h
 * @brief SpookyHash — Bob Jenkins高速非加密哈希
 *
 * 功能: 实现Bob Jenkins的SpookyHash算法，采用12状态变量混合，
 *       支持短/中/长变体，适用于哈希表和数据指纹场景。
 *
 * 协作: DataDeduplicator(数据去重) / ConsistentHash(一致性哈希)
 */
#ifndef SPOOKYHASH_H
#define SPOOKYHASH_H

#include <QObject>
#include <QByteArray>
#include <QPair>

/**
 * @brief SpookyHash哈希计算器 — Bob Jenkins算法
 */
class SpookyHash : public QObject {
    Q_OBJECT

public:
    /** @brief 运行时统计 */
    struct Stats {
        quint64 totalHashes       = 0;   ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SpookyHash(QObject* parent = nullptr);

    /**
     * @brief 设置哈希种子
     * @param seed1 第一种子
     * @param seed2 第二种子
     */
    void setSeed(quint64 seed1, quint64 seed2);

    /**
     * @brief 计算64位SpookyHash
     * @param data 输入数据
     * @return 64位哈希值
     */
    quint64 hash64(const QByteArray& data);

    /**
     * @brief 计算128位SpookyHash
     * @param data 输入数据
     * @return 128位哈希值(QPair<高位, 低位>)
     */
    QPair<quint64, quint64> hash128(const QByteArray& data);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /**
     * @brief 短消息处理 (<=192字节)
     * @param data 数据指针
     * @param len  数据长度
     * @param h1   种子1(输入/输出)
     * @param h2   种子2(输入/输出)
     */
    static void shortHash(const void* data, int len,
                          quint64& h1, quint64& h2);

    /**
     * @brief 混合12个状态变量
     * @param data 数据指针(12个quint64)
     * @param s    12个状态变量(输入/输出)
     */
    static void mix(const quint64* data, quint64* s);

    /**
     * @brief 最终混合步骤
     * @param s 12个状态变量(输入/输出)
     */
    static void end(quint64* s);

    Stats   m_stats;
    quint64 m_seed1;     ///< 第一种子
    quint64 m_seed2;     ///< 第二种子
    double  m_timeSumMs; ///< 累计耗时(用于计算均值)
};

#endif // SPOOKYHASH_H
